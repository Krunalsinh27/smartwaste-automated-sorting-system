#ifndef SMARTWASTE_API_CONTROLLERS_SORTINGAPICONTROLLER_HPP
#define SMARTWASTE_API_CONTROLLERS_SORTINGAPICONTROLLER_HPP

#include "../ApiRouter.hpp"
#include "../../engine/SmartWasteEngine.hpp"

namespace smartwaste::api::controllers {

/**
 * @brief Controller handling waste sorting REST API endpoints (/api/v1/sorting/...).
 * Translates incoming HTTP requests to SmartWasteEngine sorting operations and formats JSON responses.
 */
class SortingApiController {
public:
    static void registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine);

    static HttpResponse processWasteItem(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
    static HttpResponse getStatistics(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
};

} // namespace smartwaste::api::controllers

#endif // SMARTWASTE_API_CONTROLLERS_SORTINGAPICONTROLLER_HPP
