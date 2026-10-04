#include "SimulationController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api::controllers {

void SimulationController::registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine) {
    if (!engine) return;

    router.addRoute("POST", "/api/v1/simulation/run", [engine](const HttpRequest& req) {
        return runSimulation(engine, req);
    });

    LOG_INFO("SimulationController", "Registered simulation REST API endpoints (/api/v1/simulation/...)");
}

HttpResponse SimulationController::runSimulation(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request) {
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
        res.setJsonBody(ApiError::badRequest("Simulation request payload cannot be empty."));
        return res;
    }

    nlohmann::json reqJson;
    try {
        reqJson = nlohmann::json::parse(request.body);
    } catch (...) {
        HttpResponse res;
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest("Malformed JSON string in simulation request."));
        return res;
    }

    if (!reqJson.contains("items") || !reqJson["items"].is_array()) {
        HttpResponse res;
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest("Missing or invalid required array field 'items'."));
        return res;
    }

    std::vector<smartwaste::core::WasteItem> items;
    for (const auto& jItem : reqJson["items"]) {
        if (!jItem.is_object() || !jItem.contains("type") || !jItem["type"].is_string()) {
            continue;
        }

        smartwaste::core::WasteItem item;
        item.id = jItem.value("id", "SIM-ITEM-DEFAULT");
        item.timestampMs = jItem.value("timestampMs", static_cast<std::int64_t>(0));
        item.type = smartwaste::core::stringToWasteType(jItem["type"].get<std::string>());
        item.confidence = jItem.value("confidence", 0.0);
        item.gateId = jItem.value("gateId", 0);

        if (jItem.contains("boundingBox") && jItem["boundingBox"].is_object()) {
            auto bb = jItem["boundingBox"];
            item.boundingBox.x = bb.value("x", 0);
            item.boundingBox.y = bb.value("y", 0);
            item.boundingBox.width = bb.value("width", 0);
            item.boundingBox.height = bb.value("height", 0);
        }

        items.push_back(item);
    }

    if (items.empty()) {
        HttpResponse res;
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::badRequest("Simulation item list contains no valid WasteItem objects."));
        return res;
    }

    auto stats = engine->runSimulation(items);

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
