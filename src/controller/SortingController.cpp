#include "SortingController.hpp"
#include "../hardware/HardwareManager.hpp"
#include "../logging/Logger.hpp"
#include <sstream>

namespace smartwaste::controller {

SortingController::SortingController(
    const smartwaste::config::ConfigManager& configManager
)
    : m_configManager(configManager) {
}

int SortingController::getTargetGate(
    smartwaste::core::WasteType type
) const {
    const auto& sortingConfig = m_configManager.getSortingConfig();

    switch (type) {
        case smartwaste::core::WasteType::PLASTIC:
            return sortingConfig.plasticGate;

        case smartwaste::core::WasteType::METAL:
            return sortingConfig.metalGate;

        case smartwaste::core::WasteType::PAPER:
            return sortingConfig.paperGate;

        case smartwaste::core::WasteType::GLASS:
            return sortingConfig.glassGate;

        case smartwaste::core::WasteType::OTHER:
        case smartwaste::core::WasteType::UNKNOWN:
        default:
            return sortingConfig.defaultOtherGate;
    }
}

smartwaste::core::SortingResult
SortingController::classify(
    const smartwaste::core::WasteItem& item
) const {

    smartwaste::core::SortingResult result;

    result.type = item.type;
    result.confidence = item.confidence;

    const auto& cameraConfig = m_configManager.getCameraConfig();

    // Reject items whose detection confidence is below
    // the configured minimum threshold.
    if (item.confidence < cameraConfig.minConfidenceThreshold) {
        result.targetGate = 0;
        result.status = "REJECTED";
        result.message =
            "Waste item confidence is below the configured threshold.";

        return result;
    }

    // Unknown waste is routed to the default gate.
    result.targetGate = getTargetGate(item.type);

    if (result.targetGate <= 0) {
        result.status = "GATE_UNAVAILABLE";
        result.message =
            "No valid target gate is configured for this waste type.";

        return result;
    }

    result.status = "SUCCESS";
    result.message =
        "Waste item classified and target gate selected successfully.";

    return result;
}

bool SortingController::routeToHardware(
    smartwaste::core::SortingResult& result,
    smartwaste::hardware::HardwareManager& hwManager
) const {
    if (result.status == "REJECTED") {
        LOG_WARN("Sorting", "[SortingController] Skipping actuation: Item classification was REJECTED.");
        return false;
    }

    if (result.status == "GATE_UNAVAILABLE" || result.targetGate <= 0) {
        result.status = "GATE_UNAVAILABLE";
        std::ostringstream oss;
        oss << "[SortingController] Skipping actuation: Target gate #"
            << result.targetGate << " is UNAVAILABLE.";
        result.message = oss.str();
        LOG_WARN("Sorting", result.message);
        return false;
    }

    // 1-based Gate ID to 0-based HardwareManager vector index
    std::size_t gateIndex = static_cast<std::size_t>(result.targetGate - 1);

    if (gateIndex >= hwManager.getGateCount()) {
        result.status = "GATE_UNAVAILABLE";
        std::ostringstream oss;
        oss << "[SortingController] Actuation failed: Target gate #"
            << result.targetGate << " (index " << gateIndex
            << ") exceeds registered gate count (" << hwManager.getGateCount() << ").";
        result.message = oss.str();
        LOG_WARN("Sorting", result.message);
        return false;
    }

    bool success = hwManager.actuateGate(gateIndex);
    if (success) {
        std::ostringstream oss;
        oss << "[SortingController] Gate #" << result.targetGate << " actuated successfully.";
        result.message = oss.str();
        LOG_INFO("Sorting", result.message);
        return true;
    } else {
        result.status = "GATE_JAMMED";
        std::ostringstream oss;
        oss << "[SortingController] Actuation failed for Gate #" << result.targetGate
            << ": Mechanical jam or hardware fault detected.";
        result.message = oss.str();
        LOG_ERROR("Sorting", result.message);
        return false;
    }
}

smartwaste::core::SortingResult
SortingController::processAndRoute(
    const smartwaste::core::WasteItem& item,
    smartwaste::hardware::HardwareManager& hwManager
) const {
    smartwaste::core::SortingResult result = classify(item);
    routeToHardware(result, hwManager);
    return result;
}

} // namespace smartwaste::controller