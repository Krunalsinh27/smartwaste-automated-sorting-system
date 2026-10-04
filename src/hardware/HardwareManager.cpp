#include "HardwareManager.hpp"
#include "../logging/Logger.hpp"
#include <sstream>

namespace smartwaste::hardware {

HardwareManager::HardwareManager(
    smartwaste::hal::IConveyor& conveyor,
    smartwaste::hal::IMotor& motor
) : m_conveyor(conveyor), m_motor(motor) {}

void HardwareManager::addGate(smartwaste::hal::IGate& gate) {
    ScopedLock lock(m_mutex);
    m_gates.push_back(&gate);
    std::ostringstream oss;
    oss << "[HardwareManager] Registered diverter gate #" << m_gates.size() << " via HAL interface.";
    LOG_INFO("Hardware", oss.str());
}

void HardwareManager::addSensor(smartwaste::hal::ISensor& sensor) {
    ScopedLock lock(m_mutex);
    m_sensors.push_back(&sensor);
    std::ostringstream oss;
    oss << "[HardwareManager] Registered detection sensor #" << m_sensors.size() << " via HAL interface.";
    LOG_INFO("Hardware", oss.str());
}

bool HardwareManager::actuateGate(std::size_t gateIndex) {
    ScopedLock lock(m_mutex);
    if (gateIndex >= m_gates.size()) {
        std::ostringstream oss;
        oss << "[HardwareManager] Actuation failed: Invalid gate index " << gateIndex
            << " (Registered gates: " << m_gates.size() << ").";
        LOG_ERROR("Hardware", oss.str());
        return false;
    }

    smartwaste::hal::IGate* gate = m_gates[gateIndex];
    if (gate == nullptr) {
        LOG_ERROR("Hardware", "[HardwareManager] Actuation failed: Null gate pointer encountered.");
        return false;
    }

    if (gate->isJamDetected()) {
        std::ostringstream oss;
        oss << "[HardwareManager] Actuation REJECTED for gate index " << gateIndex << ": Mechanical jam detected!";
        LOG_WARN("Hardware", oss.str());
        return false;
    }

    gate->actuate();
    return true;
}

bool HardwareManager::isGateJammed(std::size_t gateIndex) const {
    ScopedLock lock(m_mutex);
    if (gateIndex >= m_gates.size() || m_gates[gateIndex] == nullptr) {
        return false;
    }
    return m_gates[gateIndex]->isJamDetected();
}

bool HardwareManager::hasAnyGateJam() const {
    ScopedLock lock(m_mutex);
    for (const auto* gate : m_gates) {
        if (gate != nullptr && gate->isJamDetected()) {
            return true;
        }
    }
    return false;
}

void HardwareManager::stopAll() {
    ScopedLock lock(m_mutex);
    m_conveyor.stop();
    m_motor.stop();
    LOG_INFO("Hardware", "[HardwareManager] Safety shutdown executed: Conveyor and drive motor stopped.");
}

std::size_t HardwareManager::getGateCount() const {
    ScopedLock lock(m_mutex);
    return m_gates.size();
}

std::size_t HardwareManager::getSensorCount() const {
    ScopedLock lock(m_mutex);
    return m_sensors.size();
}

smartwaste::hal::IGate* HardwareManager::getGate(std::size_t gateIndex) const {
    ScopedLock lock(m_mutex);
    if (gateIndex >= m_gates.size()) {
        return nullptr;
    }
    return m_gates[gateIndex];
}

smartwaste::hal::ISensor* HardwareManager::getSensor(std::size_t sensorIndex) const {
    ScopedLock lock(m_mutex);
    if (sensorIndex >= m_sensors.size()) {
        return nullptr;
    }
    return m_sensors[sensorIndex];
}

} // namespace smartwaste::hardware
