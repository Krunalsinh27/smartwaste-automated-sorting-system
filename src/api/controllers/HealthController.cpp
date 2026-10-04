#include "HealthController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api::controllers {

void HealthController::registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine) {
    if (!engine) return;

    router.addRoute("GET", "/api/v1/health", [engine](const HttpRequest& req) {
        return getHealth(engine, req);
    });

    LOG_INFO("HealthController", "Registered health REST API endpoints (/api/v1/health)");
}

HttpResponse HealthController::getHealth(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    auto h = engine->getHealthStatus();

    nlohmann::json data;
    data["status"] = smartwaste::hardware::healthStatusToString(h.status);
    data["message"] = h.message;
    data["isGateJammed"] = h.isGateJammed;
    data["isMotorStopped"] = h.isMotorStopped;

    nlohmann::json resJson;
    resJson["success"] = true;
    resJson["data"] = data;

    HttpResponse res;
    res.statusCode = 200;
    res.statusMessage = "OK";
    res.setJsonBody(resJson);
    return res;
}

} // namespace smartwaste::api::controllers
