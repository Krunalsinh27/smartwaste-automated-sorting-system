#include "HardwareTelemetry.hpp"
#include <sstream>
#include <iomanip>

namespace smartwaste::hardware {

std::string HardwareTelemetry::toString() const {
    std::ostringstream oss;
    oss << "[HardwareTelemetry]"
        << " Timestamp: " << timestampMs << " ms"
        << " | Conveyor: " << std::fixed << std::setprecision(2) << conveyorSpeed << " m/s"
        << " | Motor: " << std::fixed << std::setprecision(1) << motorRPM << " RPM (" << (motorRunning ? "RUNNING" : "STOPPED") << ")"
        << " | Sensor: " << std::fixed << std::setprecision(2) << sensorSignal << " (" << (sensorTriggered ? "TRIGGERED" : "CLEAR") << ")"
        << " | Gate Jam: " << (gateJamDetected ? "YES" : "NO");
    return oss.str();
}

} // namespace smartwaste::hardware
