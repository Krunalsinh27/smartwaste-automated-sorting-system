#include "TelemetryController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api::controllers {

void TelemetryController::registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine) {
    if (!engine) return;

    router.addRoute("GET", "/api/v1/telemetry/latest", [engine](const HttpRequest& req) {
        return getLatestTelemetry(engine, req);
    });

    LOG_INFO("TelemetryController", "Registered telemetry REST API endpoints (/api/v1/telemetry/...)");
}

HttpResponse TelemetryController::getLatestTelemetry(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    auto t = engine->getTelemetry();

    nlohmann::json data;
    data["timestampMs"] = t.timestampMs;
    data["conveyorSpeed"] = t.conveyorSpeed;
    data["motorRPM"] = t.motorRPM;
    data["motorRunning"] = t.motorRunning;
    data["sensorSignal"] = t.sensorSignal;
    data["sensorTriggered"] = t.sensorTriggered;
    data["gateJamDetected"] = t.gateJamDetected;

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
