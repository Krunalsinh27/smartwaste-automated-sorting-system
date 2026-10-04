#include "SimulatedConveyor.hpp"
#include "../logging/Logger.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace smartwaste::hardware {

SimulatedConveyor::SimulatedConveyor(double maxSpeed)
    : m_currentSpeed(0.0), m_maxSpeed(maxSpeed > 0.0 ? maxSpeed : 5.0) {}

void SimulatedConveyor::setSpeed(double speed) {
    ScopedLock lock(m_mutex);
    double clampedSpeed = (std::min)((std::max)(speed, 0.0), m_maxSpeed);
    m_currentSpeed = clampedSpeed;

    std::ostringstream oss;
    oss << "[SimulatedConveyor] Speed set to " << std::fixed << std::setprecision(2)
        << m_currentSpeed << " m/s (Max: " << m_maxSpeed << " m/s)";
    LOG_INFO("Hardware", oss.str());
}

double SimulatedConveyor::getSpeed() const {
    ScopedLock lock(m_mutex);
    return m_currentSpeed;
}

void SimulatedConveyor::stop() {
    ScopedLock lock(m_mutex);
    m_currentSpeed = 0.0;
    LOG_INFO("Hardware", "[SimulatedConveyor] Conveyor belt stopped (Speed: 0.0 m/s).");
}

double SimulatedConveyor::getMaxSpeed() const {
    ScopedLock lock(m_mutex);
    return m_maxSpeed;
}

bool SimulatedConveyor::isRunning() const {
    ScopedLock lock(m_mutex);
    return m_currentSpeed > 0.0;
}

} // namespace smartwaste::hardware
