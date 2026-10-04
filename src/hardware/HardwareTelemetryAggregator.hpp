#ifndef SMARTWASTE_HARDWARE_HARDWARETELEMETRYAGGREGATOR_HPP
#define SMARTWASTE_HARDWARE_HARDWARETELEMETRYAGGREGATOR_HPP

#include "../hal/IConveyor.hpp"
#include "../hal/IMotor.hpp"
#include "../hal/ISensor.hpp"
#include "../hal/IGate.hpp"
#include "HardwareTelemetry.hpp"

namespace smartwaste::hardware {

/**
 * @brief Thread-safe observer layer that aggregates live telemetry snapshot data
 * strictly through HAL pure virtual interfaces (IConveyor, IMotor, ISensor, IGate).
 * Demonstrates Dependency Inversion by depending solely on HAL abstractions.
 */
class HardwareTelemetryAggregator {
public:
    HardwareTelemetryAggregator(
        const smartwaste::hal::IConveyor& conveyor,
        const smartwaste::hal::IMotor& motor,
        const smartwaste::hal::ISensor& sensor,
        const smartwaste::hal::IGate& gate
    );

    ~HardwareTelemetryAggregator() = default;

    /**
     * @brief Performs a read-only snapshot collection of hardware telemetry.
     * @return HardwareTelemetry Structured value object of collected hardware metrics.
     */
    HardwareTelemetry read() const;

private:
    const smartwaste::hal::IConveyor& m_conveyor;
    const smartwaste::hal::IMotor& m_motor;
    const smartwaste::hal::ISensor& m_sensor;
    const smartwaste::hal::IGate& m_gate;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_HARDWARETELEMETRYAGGREGATOR_HPP
