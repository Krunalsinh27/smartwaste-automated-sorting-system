#include "SimulatedSensor.hpp"
#include "../logging/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace smartwaste::hardware {

SimulatedSensor::SimulatedSensor(double threshold)
    : m_signal(0.0), m_threshold(threshold) {}

double SimulatedSensor::readSignal() const {
    ScopedLock lock(m_mutex);
    return m_signal;
}

bool SimulatedSensor::isTriggered() const {
    ScopedLock lock(m_mutex);
    if (m_useForcedTrigger) {
        return m_forceTriggered;
    }
    return m_signal >= m_threshold;
}

void SimulatedSensor::setSignal(double signal) {
    ScopedLock lock(m_mutex);
    m_signal = signal;
    m_useForcedTrigger = false;
    
    bool active = (m_signal >= m_threshold);
    if (active) {
        std::ostringstream oss;
        oss << "[SimulatedSensor] Sensor signal changed to " << std::fixed << std::setprecision(2)
            << m_signal << " (Threshold: " << m_threshold << " -> TRIGGERED)";
        LOG_INFO("Hardware", oss.str());
    }
}

void SimulatedSensor::setTriggered(bool triggered) {
    ScopedLock lock(m_mutex);
    m_forceTriggered = triggered;
    m_useForcedTrigger = true;
    if (triggered) {
        m_signal = m_threshold;
        LOG_INFO("Hardware", "[SimulatedSensor] Sensor state manually forced to TRIGGERED.");
    } else {
        m_signal = 0.0;
        LOG_INFO("Hardware", "[SimulatedSensor] Sensor state manually forced to CLEAR.");
    }
}

void SimulatedSensor::setThreshold(double threshold) {
    ScopedLock lock(m_mutex);
    m_threshold = threshold;
}

double SimulatedSensor::getThreshold() const {
    ScopedLock lock(m_mutex);
    return m_threshold;
}

} // namespace smartwaste::hardware
