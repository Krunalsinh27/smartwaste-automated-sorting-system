#include "HttpServer.hpp"
#include "../logging/Logger.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace smartwaste::api {

struct WsaSession {
    bool initialized{false};
    WsaSession() {
#if defined(_WIN32) || defined(_WIN64)
        WSADATA wsaData;
        initialized = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
#else
        initialized = true;
#endif
    }
    ~WsaSession() {
#if defined(_WIN32) || defined(_WIN64)
        if (initialized) {
            WSACleanup();
        }
#endif
    }
};

HttpServer::HttpServer(ServerConfig config)
    : m_config(std::move(config)) {}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    smartwaste::hardware::ScopedLock lock(m_serverMutex);

    if (m_isRunning) {
        return true;
    }

    static WsaSession wsaSession;
    if (!wsaSession.initialized) {
        LOG_ERROR("HttpServer", "WSAStartup socket library initialization failed!");
        return false;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET_HANDLE) {
        LOG_ERROR("HttpServer", "Failed to create socket!");
        return false;
    }

    // Set socket option SO_REUSEADDR
    int optval = 1;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(m_config.address.c_str());
    service.sin_port = htons(static_cast<u_short>(m_config.port));

    if (bind(m_listenSocket, (struct sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
        LOG_ERROR("HttpServer", "Failed to bind socket to " + m_config.address + ":" + std::to_string(m_config.port));
        CLOSE_SOCKET(m_listenSocket);
        m_listenSocket = INVALID_SOCKET_HANDLE;
        return false;
    }

    if (listen(m_listenSocket, m_config.backlog) == SOCKET_ERROR) {
        LOG_ERROR("HttpServer", "Failed to listen on socket!");
        CLOSE_SOCKET(m_listenSocket);
        m_listenSocket = INVALID_SOCKET_HANDLE;
        return false;
    }

    m_isRunning = true;
    LOG_INFO("HttpServer", "Native C++ HTTP Server initialized and listening on http://" + m_config.address + ":" + std::to_string(m_config.port));
    return true;
}

void HttpServer::stop() {
    smartwaste::hardware::ScopedLock lock(m_serverMutex);

    if (m_listenSocket != INVALID_SOCKET_HANDLE) {
        CLOSE_SOCKET(m_listenSocket);
        m_listenSocket = INVALID_SOCKET_HANDLE;
    }
    m_isRunning = false;
    LOG_INFO("HttpServer", "HTTP Server stopped cleanly.");
}

HttpRequest HttpServer::parseRequest(const std::string& rawRequest, std::size_t maxRequestSize) {
    HttpRequest req;

    if (rawRequest.empty()) {
        req.isMalformed = true;
        return req;
    }

    std::size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = rawRequest.find("\n\n");
        if (headerEnd == std::string::npos) {
            req.isMalformed = true;
            return req;
        }
    }

    std::string headerPart = rawRequest.substr(0, headerEnd);
    std::size_t bodyStart = headerEnd + ((rawRequest.find("\r\n\r\n") != std::string::npos) ? 4 : 2);

    std::istringstream stream(headerPart);
    std::string requestLine;
    if (!std::getline(stream, requestLine)) {
        req.isMalformed = true;
        return req;
    }

    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream lineStream(requestLine);
    if (!(lineStream >> req.method >> req.path >> req.version)) {
        req.isMalformed = true;
        return req;
    }

    // Method uppercase normalization
    std::transform(req.method.begin(), req.method.end(), req.method.begin(), ::toupper);

    // Parse Headers
    std::string line;
    std::size_t contentLength = 0;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) continue;

        std::size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);

            while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) {
                val.erase(val.begin());
            }
            req.headers[key] = val;

            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
            if (lowerKey == "content-length") {
                try {
                    contentLength = std::stoul(val);
                } catch (...) {
                    contentLength = 0;
                }
            }
        }
    }

    // Check Content-Length against payload size limit before processing body
    if (contentLength > maxRequestSize) {
        req.isMalformed = true;
        req.headers["X-Payload-Too-Large"] = "true";
        return req;
    }

    if (bodyStart < rawRequest.length()) {
        req.body = rawRequest.substr(bodyStart);
        if (req.body.length() > maxRequestSize) {
            req.isMalformed = true;
            req.headers["X-Payload-Too-Large"] = "true";
        }
    }

    return req;
}

std::string HttpServer::formatResponse(const HttpResponse& response) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.statusCode << " " << response.statusMessage << "\r\n";
    for (const auto& kv : response.headers) {
        oss << kv.first << ": " << kv.second << "\r\n";
    }
    oss << "Content-Length: " << response.body.length() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << response.body;
    return oss.str();
}

bool HttpServer::processPendingConnection() {
    smartwaste::hardware::ScopedLock lock(m_serverMutex);

    if (!m_isRunning || m_listenSocket == INVALID_SOCKET_HANDLE) {
        return false;
    }

    fd_set readFds;
    FD_ZERO(&readFds);
    FD_SET(m_listenSocket, &readFds);

    timeval tv{0, 10000}; // 10 ms non-blocking timeout check
    int selectRes = select(0, &readFds, nullptr, nullptr, &tv);
    if (selectRes <= 0) {
        return false;
    }

    sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    SocketHandle clientSocket = accept(m_listenSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSocket == INVALID_SOCKET_HANDLE) {
        return false;
    }

    SocketWrapper client(clientSocket);

    std::string rawReq;
    char buffer[4096];
    int bytesRead = recv(client.handle, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        return false;
    }
    rawReq.append(buffer, static_cast<std::size_t>(bytesRead));

    // Ensure complete headers are received
    std::size_t headerEnd = rawReq.find("\r\n\r\n");
    std::size_t delimiterLen = 4;
    if (headerEnd == std::string::npos) {
        headerEnd = rawReq.find("\n\n");
        delimiterLen = 2;
    }

    while (headerEnd == std::string::npos && rawReq.length() < m_config.maxRequestSize) {
        fd_set readFdsChunk;
        FD_ZERO(&readFdsChunk);
        FD_SET(client.handle, &readFdsChunk);
        timeval tvChunk{0, 50000}; // 50 ms timeout
        int selRet = select(0, &readFdsChunk, nullptr, nullptr, &tvChunk);
        if (selRet <= 0) break;

        int chunk = recv(client.handle, buffer, sizeof(buffer) - 1, 0);
        if (chunk <= 0) break;
        rawReq.append(buffer, static_cast<std::size_t>(chunk));

        headerEnd = rawReq.find("\r\n\r\n");
        delimiterLen = 4;
        if (headerEnd == std::string::npos) {
            headerEnd = rawReq.find("\n\n");
            delimiterLen = 2;
        }
    }

    // Parse Content-Length & Expect: 100-continue if headers are present
    if (headerEnd != std::string::npos) {
        std::size_t bodyStart = headerEnd + delimiterLen;
        std::string headerPart = rawReq.substr(0, headerEnd);

        std::istringstream stream(headerPart);
        std::string line;
        std::size_t contentLength = 0;
        bool is100ContinueExpected = false;

        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());

                std::string lowerKey = key;
                std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
                if (lowerKey == "content-length") {
                    try { contentLength = std::stoul(val); } catch (...) { contentLength = 0; }
                } else if (lowerKey == "expect") {
                    std::string lowerVal = val;
                    std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), ::tolower);
                    if (lowerVal.find("100-continue") != std::string::npos) {
                        is100ContinueExpected = true;
                    }
                }
            }
        }

        // Send HTTP 100 Continue if client requested it before transmitting body
        if (is100ContinueExpected) {
            const char* continueResp = "HTTP/1.1 100 Continue\r\n\r\n";
            send(client.handle, continueResp, static_cast<int>(strlen(continueResp)), 0);
        }

        // Read remaining request body bytes if specified by Content-Length
        if (contentLength > 0 && contentLength <= m_config.maxRequestSize) {
            std::size_t targetTotalLen = bodyStart + contentLength;
            while (rawReq.length() < targetTotalLen) {
                fd_set readFdsBody;
                FD_ZERO(&readFdsBody);
                FD_SET(client.handle, &readFdsBody);
                timeval tvBody{0, 100000}; // 100 ms timeout
                int selRet = select(0, &readFdsBody, nullptr, nullptr, &tvBody);
                if (selRet <= 0) break;

                std::size_t needed = targetTotalLen - rawReq.length();
                std::size_t toRead = std::min(sizeof(buffer) - 1, needed);
                int chunk = recv(client.handle, buffer, static_cast<int>(toRead), 0);
                if (chunk <= 0) break;
                rawReq.append(buffer, static_cast<std::size_t>(chunk));
            }
        }
    }

    HttpRequest req = parseRequest(rawReq, m_config.maxRequestSize);

    HttpResponse res;
    if (req.headers.find("X-Payload-Too-Large") != req.headers.end()) {
        res.statusCode = 413;
        res.statusMessage = "Payload Too Large";
        res.setJsonBody(ApiError::payloadTooLarge());
    } else if (req.isMalformed) {
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest());
    } else {
        res = m_router.routeRequest(req);
    }

    std::string responseText = formatResponse(res);
    send(client.handle, responseText.c_str(), static_cast<int>(responseText.length()), 0);

    return true;
}


} // namespace smartwaste::api
