#ifndef SMARTWASTE_HARDWARE_SYSTEMHEALTHMONITOR_HPP
#define SMARTWASTE_HARDWARE_SYSTEMHEALTHMONITOR_HPP

#include "HardwareTelemetry.hpp"
#include <string>

namespace smartwaste::hardware {

enum class HealthStatus {
    HEALTHY,
    WARNING,
    FAULT
};

std::string healthStatusToString(HealthStatus status);

/**
 * @brief Structured value object capturing hardware health assessment results.
 */
struct HardwareHealth {
    HealthStatus status{HealthStatus::HEALTHY};
    std::string message{"System hardware operating normally."};
    bool isGateJammed{false};
    bool isMotorStopped{false};

    std::string toString() const;
};

/**
 * @brief Observer component evaluating system telemetry to detect faults and warning conditions.
 * Operates strictly on HardwareTelemetry value objects without concrete hardware dependencies.
 */
class SystemHealthMonitor {
public:
    SystemHealthMonitor() = default;

    /**
     * @brief Evaluates a HardwareTelemetry snapshot for faults or warnings.
     * @param telemetry Telemetry data captured from HAL layer.
     * @return HardwareHealth Health evaluation result.
     */
    HardwareHealth evaluate(const HardwareTelemetry& telemetry) const;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_SYSTEMHEALTHMONITOR_HPP
