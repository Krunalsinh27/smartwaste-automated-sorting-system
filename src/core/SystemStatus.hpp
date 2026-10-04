#ifndef SMARTWASTE_CORE_SYSTEMSTATUS_HPP
#define SMARTWASTE_CORE_SYSTEMSTATUS_HPP

#include "MachineState.hpp"
#include "WasteType.hpp"
#include <string>
#include <cstdint>

namespace smartwaste::core {

/**
 * @brief Represents snapshot telemetry status of the entire sorting machine.
 * Completely independent of Qt, OpenCV, or physical hardware drivers.
 */
struct SystemStatus {
    MachineState state{MachineState::IDLE};
    bool isCameraConnected{false};
    bool isConveyorRunning{false};
    bool isMotorNormal{true};
    bool isEmergencyStopActive{false};
    WasteType currentDetectedWaste{WasteType::UNKNOWN};
    std::uint64_t totalProcessedItems{0};

    std::string toString() const;
};

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_SYSTEMSTATUS_HPP
