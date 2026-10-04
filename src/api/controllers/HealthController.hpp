#ifndef SMARTWASTE_API_CONTROLLERS_HEALTHCONTROLLER_HPP
#define SMARTWASTE_API_CONTROLLERS_HEALTHCONTROLLER_HPP

#include "../ApiRouter.hpp"
#include "../../engine/SmartWasteEngine.hpp"

namespace smartwaste::api::controllers {

/**
 * @brief Controller handling system health REST API endpoints (/api/v1/health...).
 * Exposes current hardware health evaluation via SmartWasteEngine.
 */
class HealthController {
public:
    static void registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine);

    static HttpResponse getHealth(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
};

} // namespace smartwaste::api::controllers

#endif // SMARTWASTE_API_CONTROLLERS_HEALTHCONTROLLER_HPP
