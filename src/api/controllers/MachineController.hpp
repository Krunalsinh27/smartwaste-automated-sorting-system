#ifndef SMARTWASTE_API_CONTROLLERS_MACHINECONTROLLER_HPP
#define SMARTWASTE_API_CONTROLLERS_MACHINECONTROLLER_HPP

#include "../ApiRouter.hpp"
#include "../../engine/SmartWasteEngine.hpp"

namespace smartwaste::api::controllers {

/**
 * @brief Controller handling machine lifecycle REST API endpoints (/api/v1/machine/...).
 * Translates incoming HTTP requests to SmartWasteEngine operations and formats standard JSON responses.
 */
class MachineController {
public:
    static void registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine);

    static HttpResponse getStatus(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse startMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse pauseMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse resumeMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse stopMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse emergencyStop(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse resetMachine(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
};

} // namespace smartwaste::api::controllers

#endif // SMARTWASTE_API_CONTROLLERS_MACHINECONTROLLER_HPP
