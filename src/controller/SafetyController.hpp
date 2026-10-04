#ifndef SMARTWASTE_CONTROLLER_SAFETYCONTROLLER_HPP
#define SMARTWASTE_CONTROLLER_SAFETYCONTROLLER_HPP

#include "../hardware/HardwareManager.hpp"
#include "../hardware/HardwareTelemetry.hpp"
#include "../hardware/SystemHealthMonitor.hpp"

namespace smartwaste::controller {

/**
 * @brief Safety interlock controller evaluating pre-flight and runtime hardware safety.
 * Interacts strictly via HardwareManager and HardwareTelemetry abstractions.
 */
class SafetyController {
public:
    explicit SafetyController(const smartwaste::hardware::SystemHealthMonitor& healthMonitor);

    /**
     * @brief Evaluates whether machine hardware is safe to transition from IDLE to STARTING/RUNNING.
     * @param hwManager Reference to active HardwareManager.
     * @return true if no gate jams or critical faults exist, false otherwise.
     */
    bool isSafeToStart(const smartwaste::hardware::HardwareManager& hwManager) const;

    /**
     * @brief Evaluates whether machine hardware is safe to continue RUNNING.
     * @param telemetry Current hardware telemetry snapshot.
     * @return true if hardware telemetry is healthy/acceptable, false if critical fault detected.
     */
    bool isSafeToContinue(const smartwaste::hardware::HardwareTelemetry& telemetry) const;

    /**
     * @brief Evaluates whether an immediate emergency stop must be asserted due to hardware failure.
     * @param telemetry Current hardware telemetry snapshot.
     * @return true if critical fault (e.g. gate jam) requires emergency stop, false otherwise.
     */
    bool isEmergencyStopRequired(const smartwaste::hardware::HardwareTelemetry& telemetry) const;

private:
    const smartwaste::hardware::SystemHealthMonitor& m_healthMonitor;
};

} // namespace smartwaste::controller

#endif // SMARTWASTE_CONTROLLER_SAFETYCONTROLLER_HPP
