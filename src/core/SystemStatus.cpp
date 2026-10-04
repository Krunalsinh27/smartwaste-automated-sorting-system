#include "SystemStatus.hpp"
#include <sstream>

namespace smartwaste::core {

std::string SystemStatus::toString() const {
    std::ostringstream oss;
    oss << "[SystemStatus] State: " << machineStateToString(state)
        << " | Camera: " << (isCameraConnected ? "CONNECTED" : "DISCONNECTED")
        << " | Conveyor: " << (isConveyorRunning ? "RUNNING" : "STOPPED")
        << " | Motor: " << (isMotorNormal ? "OK" : "FAULT")
        << " | E-Stop: " << (isEmergencyStopActive ? "ACTIVE" : "OFF")
        << " | Current Item: " << wasteTypeToString(currentDetectedWaste)
        << " | Total Items: " << totalProcessedItems;
    return oss.str();
}

} // namespace smartwaste::core
