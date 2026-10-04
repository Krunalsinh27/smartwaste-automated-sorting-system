#ifndef SMARTWASTE_API_CONTROLLERS_SIMULATIONCONTROLLER_HPP
#define SMARTWASTE_API_CONTROLLERS_SIMULATIONCONTROLLER_HPP

#include "../ApiRouter.hpp"
#include "../../engine/SmartWasteEngine.hpp"

namespace smartwaste::api::controllers {

/**
 * @brief Controller handling batch simulation REST API endpoints (/api/v1/simulation/...).
 * Translates HTTP simulation requests into SmartWasteEngine batch simulation execution.
 */
class SimulationController {
public:
    static void registerRoutes(ApiRouter& router, smartwaste::engine::SmartWasteEngine* engine);

    static HttpResponse runSimulation(smartwaste::engine::SmartWasteEngine* engine, const HttpRequest& request);
};

} // namespace smartwaste::api::controllers

#endif // SMARTWASTE_API_CONTROLLERS_SIMULATIONCONTROLLER_HPP
