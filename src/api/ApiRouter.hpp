#ifndef SMARTWASTE_API_APIROUTER_HPP
#define SMARTWASTE_API_APIROUTER_HPP

#include "ApiError.hpp"
#include "../config/json.hpp"
#include "../hardware/HardwareLock.hpp"

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

namespace smartwaste::engine {
    class SmartWasteEngine;
}

namespace smartwaste::api {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool isMalformed{false};
};

struct HttpResponse {
    int statusCode{200};
    std::string statusMessage{"OK"};
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void setJsonBody(const nlohmann::json& j) {
        body = j.dump();
        headers["Content-Type"] = "application/json";
    }
};

using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief Thread-safe API router matching HTTP requests to endpoint handlers,
 * enforcing CORS preflight, method validation, and standard error handling.
 */
class ApiRouter {
public:
    explicit ApiRouter(smartwaste::engine::SmartWasteEngine* engine = nullptr);

    void setEngine(smartwaste::engine::SmartWasteEngine* engine);
    smartwaste::engine::SmartWasteEngine* getEngine() const noexcept { return m_engine; }

    void setCorsOrigin(std::string origin) { m_corsOrigin = std::move(origin); }
    const std::string& getCorsOrigin() const noexcept { return m_corsOrigin; }

    /**
     * @brief Registers an HTTP route handler.
     * @param method HTTP method (GET, POST, DELETE, etc.)
     * @param path Exact path (e.g. /api/v1/ping)
     * @param handler Callable request handler.
     */
    void addRoute(const std::string& method, const std::string& path, RequestHandler handler);

    /**
     * @brief Dispatches an HTTP request to the appropriate registered handler or returns error.
     */
    HttpResponse routeRequest(const HttpRequest& request) const;

private:
    void registerDefaultRoutes();
    void injectCorsHeaders(HttpResponse& response) const;

    struct RouteKey {
        std::string method;
        std::string path;

        bool operator==(const RouteKey& other) const {
            return method == other.method && path == other.path;
        }
    };

    struct RouteKeyHash {
        std::size_t operator()(const RouteKey& k) const {
            return std::hash<std::string>()(k.method) ^ (std::hash<std::string>()(k.path) << 1);
        }
    };

    smartwaste::engine::SmartWasteEngine* m_engine{nullptr};
    std::string m_corsOrigin{"http://localhost:5173"};
    std::unordered_map<RouteKey, RequestHandler, RouteKeyHash> m_routes;
    mutable smartwaste::hardware::Mutex m_routerMutex;
};

} // namespace smartwaste::api

#endif // SMARTWASTE_API_APIROUTER_HPP
