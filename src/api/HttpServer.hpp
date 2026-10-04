#ifndef SMARTWASTE_API_HTTPSERVER_HPP
#define SMARTWASTE_API_HTTPSERVER_HPP

#include "ApiRouter.hpp"
#include "../hardware/HardwareLock.hpp"
#include <string>
#include <cstddef>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketHandle;
#define INVALID_SOCKET_HANDLE INVALID_SOCKET
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
typedef int SocketHandle;
#define INVALID_SOCKET_HANDLE (-1)
#define CLOSE_SOCKET(s) close(s)
#endif

namespace smartwaste::api {

struct ServerConfig {
    std::string address{"127.0.0.1"};
    int port{8080};
    std::size_t maxRequestSize{1048576}; // 1 MB limit
    int backlog{10};
};

/**
 * @brief RAII wrapper around platform socket handles preventing resource leaks on exit paths.
 */
class SocketWrapper {
public:
    SocketWrapper() : handle(INVALID_SOCKET_HANDLE) {}
    explicit SocketWrapper(SocketHandle h) : handle(h) {}
    ~SocketWrapper() {
        if (handle != INVALID_SOCKET_HANDLE) {
            CLOSE_SOCKET(handle);
            handle = INVALID_SOCKET_HANDLE;
        }
    }
    SocketWrapper(const SocketWrapper&) = delete;
    SocketWrapper& operator=(const SocketWrapper&) = delete;

    SocketWrapper(SocketWrapper&& other) noexcept : handle(other.handle) {
        other.handle = INVALID_SOCKET_HANDLE;
    }
    SocketWrapper& operator=(SocketWrapper&& other) noexcept {
        if (this != &other) {
            if (handle != INVALID_SOCKET_HANDLE) {
                CLOSE_SOCKET(handle);
            }
            handle = other.handle;
            other.handle = INVALID_SOCKET_HANDLE;
        }
        return *this;
    }

    bool isValid() const noexcept { return handle != INVALID_SOCKET_HANDLE; }

    SocketHandle handle{INVALID_SOCKET_HANDLE};
};

/**
 * @brief Native C++ HTTP/1.1 Web Server providing light-weight, RAII-managed socket server
 * functionality. Compatible with Windows (WinSock2) and POSIX sockets.
 */
class HttpServer {
public:
    explicit HttpServer(ServerConfig config = ServerConfig{});
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    ApiRouter& getRouter() noexcept { return m_router; }
    const ApiRouter& getRouter() const noexcept { return m_router; }

    /**
     * @brief Starts server listening socket.
     * @return true if port bound and server listening, false on error.
     */
    bool start();

    /**
     * @brief Accepts incoming pending client connections (non-blocking / single-step).
     * @return true if a request was accepted and served, false if no pending connection.
     */
    bool processPendingConnection();

    /**
     * @brief Stops server and cleans up listening sockets.
     */
    void stop();

    bool isRunning() const noexcept { return m_isRunning; }
    const ServerConfig& getConfig() const noexcept { return m_config; }

    /**
     * @brief Helper to parse raw HTTP request string into HttpRequest object.
     */
    static HttpRequest parseRequest(const std::string& rawRequest, std::size_t maxRequestSize);

    /**
     * @brief Helper to format HttpResponse into HTTP/1.1 response text stream.
     */
    static std::string formatResponse(const HttpResponse& response);

private:
    ServerConfig m_config;
    ApiRouter m_router;
    bool m_isRunning{false};

    SocketHandle m_listenSocket{INVALID_SOCKET_HANDLE};
    mutable smartwaste::hardware::Mutex m_serverMutex;
};

} // namespace smartwaste::api

#endif // SMARTWASTE_API_HTTPSERVER_HPP
