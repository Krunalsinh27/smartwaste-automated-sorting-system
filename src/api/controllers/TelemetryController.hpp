#ifndef SMARTWASTE_API_CONTROLLERS_TELEMETRYCONTROLLER_HPP
#define SMARTWASTE_API_CONTROLLERS_TELEMETRYCONTROLLER_HPP

#include "../ApiRouter.hpp"
#include "../../engine/SmartWasteEngine.hpp"

namespace smartwaste::api::controllers {

/**
 * @brief Controller handling telemetry REST API endpoints (/api/v1/telemetry/...).
 * Exposes latest collected hardware telemetry via SmartWasteEngine.
 */
class TelemetryController {
public:
    static void registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine);

    static HttpResponse getLatestTelemetry(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
};

} // namespace smartwaste::api::controllers

#endif // SMARTWASTE_API_CONTROLLERS_TELEMETRYCONTROLLER_HPP
