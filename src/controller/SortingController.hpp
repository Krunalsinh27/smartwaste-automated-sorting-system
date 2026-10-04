#ifndef SMARTWASTE_CONTROLLER_SORTINGCONTROLLER_HPP
#define SMARTWASTE_CONTROLLER_SORTINGCONTROLLER_HPP

#include "../core/WasteItem.hpp"
#include "../core/SortingResult.hpp"
#include "../config/ConfigManager.hpp"

namespace smartwaste::hardware {
    class HardwareManager;
}

namespace smartwaste::controller {

/**
 * @brief Decides the destination gate for a detected waste item and routes decisions to hardware.
 *
 * SortingController contains the business logic for mapping a WasteType
 * to the configured sorting gate.
 *
 * It routes actuation commands to HardwareManager via HAL abstractions.
 */
class SortingController {
public:
    explicit SortingController(
        const smartwaste::config::ConfigManager& configManager
    );

    /**
     * @brief Creates a sorting decision for a detected waste item.
     *
     * @param item Detected waste item.
     * @return SortingResult containing target gate and status.
     */
    smartwaste::core::SortingResult
    classify(const smartwaste::core::WasteItem& item) const;

    /**
     * @brief Routes a sorting result decision to the hardware manager for gate actuation.
     *
     * @param result Sorting result to route (updated if target gate is unavailable or jammed).
     * @param hwManager Hardware manager instance controlling physical/simulated gates.
     * @return true if gate actuation succeeded, false otherwise.
     */
    bool routeToHardware(
        smartwaste::core::SortingResult& result,
        smartwaste::hardware::HardwareManager& hwManager
    ) const;

    /**
     * @brief Classifies a waste item and immediately routes the decision to hardware.
     *
     * @param item Detected waste item.
     * @param hwManager Hardware manager instance controlling physical/simulated gates.
     * @return SortingResult updated with final actuation status and message.
     */
    smartwaste::core::SortingResult
    processAndRoute(
        const smartwaste::core::WasteItem& item,
        smartwaste::hardware::HardwareManager& hwManager
    ) const;

private:
    int getTargetGate(smartwaste::core::WasteType type) const;

    const smartwaste::config::ConfigManager& m_configManager;
};

} // namespace smartwaste::controller

#endif // SMARTWASTE_CONTROLLER_SORTINGCONTROLLER_HPP