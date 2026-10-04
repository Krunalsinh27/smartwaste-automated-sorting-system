#include "MachineController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::api::controllers {

void MachineController::registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine) {
    if (!engine) return;

    router.addRoute("GET", "/api/v1/machine/status", [engine](const HttpRequest& req) {
        return getStatus(engine, req);
    });

    router.addRoute("POST", "/api/v1/machine/start", [engine](const HttpRequest& req) {
        return startMachine(engine, req);
    });

    router.addRoute("POST", "/api/v1/machine/pause", [engine](const HttpRequest& req) {
        return pauseMachine(engine, req);
    });

    router.addRoute("POST", "/api/v1/machine/resume", [engine](const HttpRequest& req) {
        return resumeMachine(engine, req);
    });

    router.addRoute("POST", "/api/v1/machine/stop", [engine](const HttpRequest& req) {
        return stopMachine(engine, req);
    });

    router.addRoute("POST", "/api/v1/machine/emergency-stop", [engine](const HttpRequest& req) {
        return emergencyStop(engine, req);
    });

    router.addRoute("POST", "/api/v1/machine/reset", [engine](const HttpRequest& req) {
        return resetMachine(engine, req);
    });

    LOG_INFO("MachineController", "Registered machine lifecycle REST API endpoints (/api/v1/machine/...)");
}

HttpResponse MachineController::getStatus(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    smartwaste::core::SystemStatus status = engine->getSystemStatus();
    nlohmann::json data;
    data["state"] = smartwaste::core::machineStateToString(status.state);
    data["isCameraConnected"] = status.isCameraConnected;
    data["isConveyorRunning"] = status.isConveyorRunning;
    data["isMotorNormal"] = status.isMotorNormal;
    data["isEmergencyStopActive"] = status.isEmergencyStopActive;
    data["currentDetectedWaste"] = smartwaste::core::wasteTypeToString(status.currentDetectedWaste);
    data["totalProcessedItems"] = status.totalProcessedItems;

    nlohmann::json resJson;
    resJson["success"] = true;
    resJson["data"] = data;

    HttpResponse res;
    res.statusCode = 200;
    res.statusMessage = "OK";
    res.setJsonBody(resJson);
    return res;
}

HttpResponse MachineController::startMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    std::string curStateStr = engine->getMachineStateString();
    bool ok = engine->startMachine();
    HttpResponse res;
    if (ok) {
        res.statusCode = 200;
        res.statusMessage = "OK";
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "Machine started successfully";
        j["data"] = {{"state", engine->getMachineStateString()}};
        res.setJsonBody(j);
    } else {
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::invalidStateTransition(
            "Machine transition failed: Cannot start from state " + curStateStr));
    }
    return res;
}

HttpResponse MachineController::pauseMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    std::string curStateStr = engine->getMachineStateString();
    bool ok = engine->pauseMachine();
    HttpResponse res;
    if (ok) {
        res.statusCode = 200;
        res.statusMessage = "OK";
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "Machine paused successfully";
        j["data"] = {{"state", engine->getMachineStateString()}};
        res.setJsonBody(j);
    } else {
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::invalidStateTransition(
            "Machine transition failed: Cannot pause from state " + curStateStr));
    }
    return res;
}

HttpResponse MachineController::resumeMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    std::string curStateStr = engine->getMachineStateString();
    bool ok = engine->resumeMachine();
    HttpResponse res;
    if (ok) {
        res.statusCode = 200;
        res.statusMessage = "OK";
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "Machine resumed successfully";
        j["data"] = {{"state", engine->getMachineStateString()}};
        res.setJsonBody(j);
    } else {
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::invalidStateTransition(
            "Machine transition failed: Cannot resume from state " + curStateStr));
    }
    return res;
}

HttpResponse MachineController::stopMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    std::string curStateStr = engine->getMachineStateString();
    bool ok = engine->stopMachine();
    HttpResponse res;
    if (ok) {
        res.statusCode = 200;
        res.statusMessage = "OK";
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "Machine stopped successfully";
        j["data"] = {{"state", engine->getMachineStateString()}};
        res.setJsonBody(j);
    } else {
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::invalidStateTransition(
            "Machine transition failed: Cannot stop from state " + curStateStr));
    }
    return res;
}

HttpResponse MachineController::emergencyStop(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    engine->emergencyStop();
    HttpResponse res;
    res.statusCode = 200;
    res.statusMessage = "OK";
    nlohmann::json j;
    j["success"] = true;
    j["message"] = "Emergency stop asserted";
    j["data"] = {{"state", engine->getMachineStateString()}};
    res.setJsonBody(j);
    return res;
}

HttpResponse MachineController::resetMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest&) {
    if (!engine) {
        HttpResponse res;
        res.statusCode = 500;
        res.statusMessage = "Internal Server Error";
        res.setJsonBody(ApiError::internalServerError("SmartWasteEngine instance is null."));
        return res;
    }

    std::string curStateStr = engine->getMachineStateString();
    bool ok = engine->resetMachine();
    HttpResponse res;
    if (ok) {
        res.statusCode = 200;
        res.statusMessage = "OK";
        nlohmann::json j;
        j["success"] = true;
        j["message"] = "Machine reset successfully";
        j["data"] = {{"state", engine->getMachineStateString()}};
        res.setJsonBody(j);
    } else {
        res.statusCode = 400;
        res.statusMessage = "Bad Request";
        res.setJsonBody(ApiError::invalidStateTransition(
            "Machine transition failed: Cannot reset from state " + curStateStr));
    }
    return res;
}

} // namespace smartwaste::api::controllers
