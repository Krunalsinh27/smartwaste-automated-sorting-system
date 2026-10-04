#include "HardwareTelemetryAggregator.hpp"
#include <chrono>

namespace smartwaste::hardware {

HardwareTelemetryAggregator::HardwareTelemetryAggregator(
    const smartwaste::hal::IConveyor& conveyor,
    const smartwaste::hal::IMotor& motor,
    const smartwaste::hal::ISensor& sensor,
    const smartwaste::hal::IGate& gate
) : m_conveyor(conveyor), m_motor(motor), m_sensor(sensor), m_gate(gate) {}

HardwareTelemetry HardwareTelemetryAggregator::read() const {
    HardwareTelemetry t;
    
    t.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Read strictly through HAL interface abstractions
    t.conveyorSpeed = m_conveyor.getSpeed();
    t.motorRPM = m_motor.getRPM();
    t.motorRunning = (t.motorRPM > 0.0); // Inferred from getRPM() without altering IMotor
    t.sensorSignal = m_sensor.readSignal();
    t.sensorTriggered = m_sensor.isTriggered();
    t.gateJamDetected = m_gate.isJamDetected();

    return t;
}

} // namespace smartwaste::hardware
