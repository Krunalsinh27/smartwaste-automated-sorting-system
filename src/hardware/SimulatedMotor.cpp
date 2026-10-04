#include "SimulatedMotor.hpp"
#include "../logging/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace smartwaste::hardware {

SimulatedMotor::SimulatedMotor(double targetRPM)
    : m_isRunning(false), m_currentRPM(0.0), m_targetRPM(targetRPM > 0.0 ? targetRPM : 1500.0) {}

void SimulatedMotor::start() {
    ScopedLock lock(m_mutex);
    m_isRunning = true;
    if (m_isAbnormal) {
        m_currentRPM = 0.0;
        m_temperatureC = 95.0; // Simulated overheating
        LOG_WARN("Hardware", "[SimulatedMotor] Motor start attempted during ABNORMAL FAULT state! RPM remains 0.0.");
    } else {
        m_currentRPM = m_targetRPM;
        m_temperatureC = 45.0; // Normal operating temp
        std::ostringstream oss;
        oss << "[SimulatedMotor] Motor started. Target RPM reached: " << std::fixed << std::setprecision(1) << m_currentRPM;
        LOG_INFO("Hardware", oss.str());
    }
}

void SimulatedMotor::stop() {
    ScopedLock lock(m_mutex);
    m_isRunning = false;
    m_currentRPM = 0.0;
    m_temperatureC = 25.0;
    LOG_INFO("Hardware", "[SimulatedMotor] Motor powered down. RPM: 0.0.");
}

double SimulatedMotor::getRPM() const {
    ScopedLock lock(m_mutex);
    return m_currentRPM;
}

bool SimulatedMotor::isRunning() const {
    ScopedLock lock(m_mutex);
    return m_isRunning;
}

double SimulatedMotor::getTargetRPM() const {
    ScopedLock lock(m_mutex);
    return m_targetRPM;
}

void SimulatedMotor::setTargetRPM(double targetRPM) {
    ScopedLock lock(m_mutex);
    m_targetRPM = targetRPM > 0.0 ? targetRPM : 1500.0;
    if (m_isRunning && !m_isAbnormal) {
        m_currentRPM = m_targetRPM;
    }
}

void SimulatedMotor::setAbnormalState(bool abnormal) {
    ScopedLock lock(m_mutex);
    m_isAbnormal = abnormal;
    if (m_isAbnormal) {
        m_currentRPM = 0.0;
        m_temperatureC = 95.0;
        LOG_WARN("Hardware", "[SimulatedMotor] Fault injected: Motor set to ABNORMAL state.");
    } else {
        m_temperatureC = 25.0;
        LOG_INFO("Hardware", "[SimulatedMotor] Fault cleared: Motor returned to normal operational state.");
    }
}

bool SimulatedMotor::isAbnormalState() const {
    ScopedLock lock(m_mutex);
    return m_isAbnormal;
}

double SimulatedMotor::getTemperature() const {
    ScopedLock lock(m_mutex);
    return m_temperatureC;
}

} // namespace smartwaste::hardware
