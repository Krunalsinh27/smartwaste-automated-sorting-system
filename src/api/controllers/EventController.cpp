#include "EventController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api::controllers {

void EventController::registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine) {
    if (!engine) return;

    router.addRoute("GET", "/api/v1/events", [engine](const HttpRequest& req) {
        return getRecentEvents(engine, req);
    });

    LOG_INFO("EventController", "Registered operational event REST API endpoints (/api/v1/events)");
}

HttpResponse EventController::getRecentEvents(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    auto events = engine->getRecentEvents(20);

    nlohmann::json eventArray = nlohmann::json::array();
    for (const auto& evt : events) {
        nlohmann::json item;
        item["eventId"] = evt.eventId;
        item["timestampMs"] = evt.timestampMs;
        item["type"] = smartwaste::core::eventTypeToString(evt.type);
        item["severity"] = smartwaste::logging::logLevelToString(evt.severity);
        item["message"] = evt.message;
        item["itemId"] = evt.itemId;
        item["wasteType"] = smartwaste::core::wasteTypeToString(evt.wasteType);
        item["gateId"] = evt.gateId;
        eventArray.push_back(item);
    }

    nlohmann::json resJson;
    resJson["success"] = true;
    resJson["data"] = eventArray;

    HttpResponse res;
    res.statusCode = 200;
    res.statusMessage = "OK";
    res.setJsonBody(resJson);
    return res;
}

} // namespace smartwaste::api::controllers
