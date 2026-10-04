#include "ApiRouter.hpp"
#include "controllers/MachineController.hpp"
#include "controllers/SortingApiController.hpp"
#include "controllers/TelemetryController.hpp"
#include "controllers/HealthController.hpp"
#include "controllers/EventController.hpp"
#include "controllers/SimulationController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api {

ApiRouter::ApiRouter(smartwaste::engine::SmartWasteEngine* engine)
    : m_engine(engine) {
    registerDefaultRoutes();
}

void ApiRouter::setEngine(smartwaste::engine::SmartWasteEngine* engine) {
    m_engine = engine;
    if (m_engine) {
        controllers::MachineController::registerRoutes(*this, m_engine);
        controllers::SortingApiController::registerRoutes(*this, m_engine);
        controllers::TelemetryController::registerRoutes(*this, m_engine);
        controllers::HealthController::registerRoutes(*this, m_engine);
        controllers::EventController::registerRoutes(*this, m_engine);
        controllers::SimulationController::registerRoutes(*this, m_engine);
    }
}

void ApiRouter::registerDefaultRoutes() {
    // Registered foundation test route GET /api/v1/ping
    addRoute("GET", "/api/v1/ping", [](const HttpRequest&) {
        HttpResponse res;
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "SmartWaste API is running";
        res.setJsonBody(j);
        return res;
    });

    if (m_engine) {
        controllers::MachineController::registerRoutes(*this, m_engine);
        controllers::SortingApiController::registerRoutes(*this, m_engine);
        controllers::TelemetryController::registerRoutes(*this, m_engine);
        controllers::HealthController::registerRoutes(*this, m_engine);
        controllers::EventController::registerRoutes(*this, m_engine);
        controllers::SimulationController::registerRoutes(*this, m_engine);
    }
}

void ApiRouter::addRoute(const std::string& method, const std::string& path, RequestHandler handler) {
    smartwaste::hardware::ScopedLock lock(m_routerMutex);
    RouteKey key{method, path};
    m_routes[key] = std::move(handler);
    LOG_INFO("ApiRouter", "Registered route: " + method + " " + path);
}

void ApiRouter::injectCorsHeaders(HttpResponse& response) const {
    response.headers["Access-Control-Allow-Origin"] = m_corsOrigin;
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, DELETE, OPTIONS";
    response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
}

HttpResponse ApiRouter::routeRequest(const HttpRequest& request) const {
    smartwaste::hardware::ScopedLock lock(m_routerMutex);

    // 1. Handle OPTIONS preflight CORS request
    if (request.method == "OPTIONS") {
        HttpResponse res;
        res.statusCode = 200;
        res.statusMessage = "OK";
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "CORS preflight allowed.";
        res.setJsonBody(j);
        injectCorsHeaders(res);
        return res;
    }

    // 2. Search for exact route match (method + path)
    RouteKey targetKey{request.method, request.path};
    auto it = m_routes.find(targetKey);
    if (it != m_routes.end()) {
        HttpResponse res = it->second(request);
        injectCorsHeaders(res);
        return res;
    }

    // 3. Check if path exists under a different HTTP method (405 Method Not Allowed)
    bool pathExists = false;
    for (const auto& kv : m_routes) {
        if (kv.first.path == request.path) {
            pathExists = true;
            break;
        }
    }

    HttpResponse errRes;
    if (pathExists) {
        errRes.statusCode = 405;
        errRes.statusMessage = "Method Not Allowed";
        errRes.setJsonBody(ApiError::methodNotAllowed("HTTP method " + request.method + " is not allowed for " + request.path));
        LOG_WARN("ApiRouter", "405 Method Not Allowed for route: " + request.method + " " + request.path);
    } else {
        errRes.statusCode = 404;
        errRes.statusMessage = "Not Found";
        errRes.setJsonBody(ApiError::notFound("API endpoint not found: " + request.path));
        LOG_WARN("ApiRouter", "404 Not Found for route: " + request.method + " " + request.path);
    }

    injectCorsHeaders(errRes);
    return errRes;
}

} // namespace smartwaste::api
