#include "SystemHealthMonitor.hpp"
#include <sstream>

namespace smartwaste::hardware {

std::string healthStatusToString(HealthStatus status) {
    switch (status) {
        case HealthStatus::HEALTHY: return "HEALTHY";
        case HealthStatus::WARNING: return "WARNING";
        case HealthStatus::FAULT:   return "FAULT";
        default:                    return "UNKNOWN";
    }
}

std::string HardwareHealth::toString() const {
    std::ostringstream oss;
    oss << "[HardwareHealth] Status: " << healthStatusToString(status)
        << " | Details: " << message;
    return oss.str();
}

HardwareHealth SystemHealthMonitor::evaluate(const HardwareTelemetry& telemetry) const {
    HardwareHealth health;

    // Rule 1: Mechanical Gate Jam is a critical FAULT
    if (telemetry.gateJamDetected) {
        health.status = HealthStatus::FAULT;
        health.isGateJammed = true;
        health.message = "CRITICAL FAULT: Mechanical gate jam detected in diverter gate assembly!";
        return health;
    }

    // Rule 2: Stopped motor/conveyor condition is a WARNING
    if (!telemetry.motorRunning || telemetry.conveyorSpeed == 0.0) {
        health.status = HealthStatus::WARNING;
        health.isMotorStopped = true;
        health.message = "WARNING: Conveyor belt / drive motor is currently stopped.";
        return health;
    }

    // Rule 3: Otherwise healthy
    health.status = HealthStatus::HEALTHY;
    health.message = "OK: All hardware components operating within normal telemetry parameters.";
    return health;
}

} // namespace smartwaste::hardware
