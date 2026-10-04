#include "SortingApiController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api::controllers {

void SortingApiController::registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine) {
    if (!engine) return;

    router.addRoute("POST", "/api/v1/sorting/process", [engine](const HttpRequest& req) {
        return processWasteItem(engine, req);
    });

    router.addRoute("GET", "/api/v1/sorting/stats", [engine](const HttpRequest& req) {
        return getStatistics(engine, req);
    });

    LOG_INFO("SortingApiController", "Registered sorting REST API endpoints (/api/v1/sorting/...)");
}

HttpResponse SortingApiController::processWasteItem(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    if (request.body.empty()) {
        HttpResponse res;
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest("Request payload cannot be empty."));
        return res;
    }

    nlohmann::json reqJson;
    try {
        reqJson = nlohmann::json::parse(request.body);
    } catch (...) {
        HttpResponse res;
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest("Malformed JSON payload string."));
        return res;
    }

    if (!reqJson.contains("type") || !reqJson["type"].is_string()) {
        HttpResponse res;
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest("Missing or invalid required field 'type'."));
        return res;
    }

    smartwaste::core::WasteItem item;
    item.id = reqJson.value("id", "ITEM-API-DEFAULT");
    item.timestampMs = reqJson.value("timestampMs", static_cast<std::int64_t>(0));
    item.type = smartwaste::core::stringToWasteType(reqJson["type"].get<std::string>());
    item.confidence = reqJson.value("confidence", 0.0);
    item.gateId = reqJson.value("gateId", 0);

    if (reqJson.contains("boundingBox") && reqJson["boundingBox"].is_object()) {
        auto bb = reqJson["boundingBox"];
        item.boundingBox.x = bb.value("x", 0);
        item.boundingBox.y = bb.value("y", 0);
        item.boundingBox.width = bb.value("width", 0);
        item.boundingBox.height = bb.value("height", 0);
    }

    smartwaste::core::SortingResult result = engine->processWasteItem(item);

    nlohmann::json data;
    data["type"] = smartwaste::core::wasteTypeToString(result.type);
    data["targetGate"] = result.targetGate;
    data["confidence"] = result.confidence;
    data["status"] = result.status;
    data["message"] = result.message;

    nlohmann::json resJson;
    resJson["success"] = (result.status == "SUCCESS");
    resJson["data"] = data;

    HttpResponse res;
    res.statusCode = 200;
    res.statusMessage = "OK";
    res.setJsonBody(resJson);
    return res;
}

HttpResponse SortingApiController::getStatistics(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    auto stats = engine->getSimulationStatistics();

    nlohmann::json data;
    data["totalItems"] = stats.totalItems;
    data["successfulItems"] = stats.successfulItems;
    data["rejectedItems"] = stats.rejectedItems;
    data["unavailableGateItems"] = stats.unavailableGateItems;
    data["failedItems"] = stats.failedItems;
    data["totalGateActuations"] = stats.totalGateActuations;
    data["healthChecksPerformed"] = stats.healthChecksPerformed;
    data["healthyChecks"] = stats.healthyChecks;
    data["warningChecks"] = stats.warningChecks;
    data["criticalChecks"] = stats.criticalChecks;

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
