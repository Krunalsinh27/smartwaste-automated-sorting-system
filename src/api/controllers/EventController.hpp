#ifndef SMARTWASTE_API_CONTROLLERS_EVENTCONTROLLER_HPP
#define SMARTWASTE_API_CONTROLLERS_EVENTCONTROLLER_HPP

#include "../ApiRouter.hpp"
#include "../../engine/SmartWasteEngine.hpp"

namespace smartwaste::api::controllers {

/**
 * @brief Controller handling operational events REST API endpoints (/api/v1/events...).
 * Exposes event history records retrieved via SmartWasteEngine.
 */
class EventController {
public:
    static void registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine);

    static HttpResponse getRecentEvents(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
};

} // namespace smartwaste::api::controllers

#endif // SMARTWASTE_API_CONTROLLERS_EVENTCONTROLLER_HPP
