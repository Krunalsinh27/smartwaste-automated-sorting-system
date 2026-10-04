#include "SafetyController.hpp"
#include "../logging/Logger.hpp"

namespace smartwaste::controller {

SafetyController::SafetyController(const smartwaste::hardware::SystemHealthMonitor& healthMonitor)
    : m_healthMonitor(healthMonitor) {}

bool SafetyController::isSafeToStart(const smartwaste::hardware::HardwareManager& hwManager) const {
    if (hwManager.hasAnyGateJam()) {
        LOG_ERROR("Safety", "[SafetyController] Pre-flight start check FAILED: Gate jam detected on registered hardware!");
        return false;
    }
    LOG_INFO("Safety", "[SafetyController] Pre-flight start check PASSED: All hardware components clear for startup.");
    return true;
}

bool SafetyController::isSafeToContinue(const smartwaste::hardware::HardwareTelemetry& telemetry) const {
    auto health = m_healthMonitor.evaluate(telemetry);
    if (health.status == smartwaste::hardware::HealthStatus::FAULT) {
        LOG_ERROR("Safety", "[SafetyController] Runtime safety check FAILED: " + health.message);
        return false;
    }
    return true;
}

bool SafetyController::isEmergencyStopRequired(const smartwaste::hardware::HardwareTelemetry& telemetry) const {
    auto health = m_healthMonitor.evaluate(telemetry);
    return health.status == smartwaste::hardware::HealthStatus::FAULT;
}

} // namespace smartwaste::controller
