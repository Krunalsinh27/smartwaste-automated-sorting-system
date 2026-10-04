#include "SimulatedGate.hpp"
#include "../logging/Logger.hpp"
#include <sstream>

namespace smartwaste::hardware {

SimulatedGate::SimulatedGate(int gateId)
    : m_gateId(gateId) {}

void SimulatedGate::actuate() {
    ScopedLock lock(m_mutex);
    if (m_isJammed) {
        std::ostringstream oss;
        oss << "[SimulatedGate #" << m_gateId << "] Actuation REJECTED: Diverter gate is JAMMED!";
        LOG_WARN("Hardware", oss.str());
        return;
    }

    m_actuationCount++;
    m_wasActuated = true;

    std::ostringstream oss;
    oss << "[SimulatedGate #" << m_gateId << "] Pneumatic gate actuated cleanly (Total count: " << m_actuationCount << ").";
    LOG_INFO("Hardware", oss.str());
}

bool SimulatedGate::isJamDetected() const {
    ScopedLock lock(m_mutex);
    return m_isJammed;
}

int SimulatedGate::getGateId() const {
    ScopedLock lock(m_mutex);
    return m_gateId;
}

int SimulatedGate::getActuationCount() const {
    ScopedLock lock(m_mutex);
    return m_actuationCount;
}

bool SimulatedGate::wasActuated() const {
    ScopedLock lock(m_mutex);
    return m_wasActuated;
}

void SimulatedGate::resetActuatedFlag() {
    ScopedLock lock(m_mutex);
    m_wasActuated = false;
}

void SimulatedGate::setJamDetected(bool jammed) {
    ScopedLock lock(m_mutex);
    m_isJammed = jammed;
    std::ostringstream oss;
    if (jammed) {
        oss << "[SimulatedGate #" << m_gateId << "] FAULT INJECTED: Gate mechanical jam detected!";
        LOG_ERROR("Hardware", oss.str());
    } else {
        oss << "[SimulatedGate #" << m_gateId << "] FAULT CLEARED: Gate jam resolved.";
        LOG_INFO("Hardware", oss.str());
    }
}

void SimulatedGate::clearJam() {
    setJamDetected(false);
}

} // namespace smartwaste::hardware
