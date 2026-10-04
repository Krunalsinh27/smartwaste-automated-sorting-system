#ifndef SMARTWASTE_HARDWARE_HARDWARETELEMETRY_HPP
#define SMARTWASTE_HARDWARE_HARDWARETELEMETRY_HPP

#include <string>
#include <cstdint>

namespace smartwaste::hardware {

/**
 * @brief Read-only value object representing telemetry snapshot collected
 * strictly through HAL pure virtual interfaces (IConveyor, IMotor, ISensor, IGate).
 */
struct HardwareTelemetry {
    std::int64_t timestampMs{0};
    double conveyorSpeed{0.0};      // Obtained via IConveyor::getSpeed()
    double motorRPM{0.0};           // Obtained via IMotor::getRPM()
    bool motorRunning{false};       // Inferred from (motorRPM > 0.0) without altering IMotor
    double sensorSignal{0.0};       // Obtained via ISensor::readSignal()
    bool sensorTriggered{false};    // Obtained via ISensor::isTriggered()
    bool gateJamDetected{false};    // Obtained via IGate::isJamDetected()

    std::string toString() const;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_HARDWARETELEMETRY_HPP
