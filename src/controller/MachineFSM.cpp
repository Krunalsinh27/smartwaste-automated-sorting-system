#include "MachineFSM.hpp"
#include "../core/EventHistory.hpp"
#include "../logging/Logger.hpp"
#include <sstream>

namespace smartwaste::controller {

using smartwaste::core::MachineState;
using smartwaste::core::machineStateToString;
using smartwaste::core::EventType;
using smartwaste::logging::LogLevel;

MachineFSM::MachineFSM(
    smartwaste::hardware::HardwareManager& hwManager,
    const SafetyController& safetyController,
    smartwaste::core::EventHistory* eventHistory
) : m_state(MachineState::IDLE),
    m_hwManager(hwManager),
    m_safetyController(safetyController),
    m_eventHistory(eventHistory) {}

void MachineFSM::setEventHistory(smartwaste::core::EventHistory* eventHistory) noexcept {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    m_eventHistory = eventHistory;
}

MachineState MachineFSM::getCurrentState() const {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    return m_state;
}

std::string MachineFSM::getStateString() const {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    return machineStateToString(m_state);
}

bool MachineFSM::isValidTransition(MachineState from, MachineState to) const {
    if (to == MachineState::EMERGENCY_STOP) {
        return true; // Emergency Stop can be asserted from any state
    }

    switch (from) {
        case MachineState::IDLE:
            return (to == MachineState::STARTING || to == MachineState::ERROR);

        case MachineState::STARTING:
            return (to == MachineState::RUNNING || to == MachineState::ERROR || to == MachineState::STOPPING);

        case MachineState::RUNNING:
            return (to == MachineState::PAUSED || to == MachineState::STOPPING || to == MachineState::ERROR);

        case MachineState::PAUSED:
            return (to == MachineState::RUNNING || to == MachineState::STOPPING || to == MachineState::ERROR);

        case MachineState::STOPPING:
            return (to == MachineState::IDLE || to == MachineState::ERROR);

        case MachineState::ERROR:
            return (to == MachineState::IDLE);

        case MachineState::EMERGENCY_STOP:
            return (to == MachineState::IDLE);

        default:
            return false;
    }
}

bool MachineFSM::start() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    if (m_state != MachineState::IDLE) {
        std::ostringstream oss;
        oss << "[MachineFSM] Invalid start request from state " << machineStateToString(m_state) << ". Must be IDLE.";
        LOG_WARN("FSM", oss.str());
        return false;
    }

    // Transition IDLE -> STARTING
    m_state = MachineState::STARTING;
    LOG_INFO("FSM", "[MachineFSM] State transition: IDLE -> STARTING.");

    // Evaluate Pre-flight Safety Interlocks
    if (!m_safetyController.isSafeToStart(m_hwManager)) {
        LOG_ERROR("FSM", "[MachineFSM] Pre-flight safety check failed! Transitioning: STARTING -> ERROR.");
        m_state = MachineState::ERROR;
        m_hwManager.stopAll();
        return false;
    }

    // Engage Hardware
    m_hwManager.getMotor().start();
    m_hwManager.getConveyor().setSpeed(2.0); // Normal operating speed

    // Transition STARTING -> RUNNING
    m_state = MachineState::RUNNING;
    LOG_INFO("FSM", "[MachineFSM] Hardware spooled up successfully. State transition: STARTING -> RUNNING.");
    if (m_eventHistory != nullptr) {
        m_eventHistory->addEvent(EventType::MACHINE_STARTED, LogLevel::LOG_INFO_LEVEL, "Machine started and spooled up to RUNNING state.");
    }
    return true;
}

bool MachineFSM::pause() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    if (m_state != MachineState::RUNNING) {
        std::ostringstream oss;
        oss << "[MachineFSM] Invalid pause request from state " << machineStateToString(m_state) << ". Must be RUNNING.";
        LOG_WARN("FSM", oss.str());
        return false;
    }

    m_hwManager.getConveyor().stop();
    m_state = MachineState::PAUSED;
    LOG_INFO("FSM", "[MachineFSM] State transition: RUNNING -> PAUSED.");
    return true;
}

bool MachineFSM::resume() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    if (m_state != MachineState::PAUSED) {
        std::ostringstream oss;
        oss << "[MachineFSM] Invalid resume request from state " << machineStateToString(m_state) << ". Must be PAUSED.";
        LOG_WARN("FSM", oss.str());
        return false;
    }

    if (!m_safetyController.isSafeToStart(m_hwManager)) {
        LOG_ERROR("FSM", "[MachineFSM] Safety check failed on resume! Transitioning: PAUSED -> ERROR.");
        m_state = MachineState::ERROR;
        m_hwManager.stopAll();
        return false;
    }

    m_hwManager.getConveyor().setSpeed(2.0);
    m_state = MachineState::RUNNING;
    LOG_INFO("FSM", "[MachineFSM] State transition: PAUSED -> RUNNING.");
    return true;
}

bool MachineFSM::stop() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    if (m_state != MachineState::RUNNING && m_state != MachineState::PAUSED) {
        std::ostringstream oss;
        oss << "[MachineFSM] Invalid stop request from state " << machineStateToString(m_state) << ". Must be RUNNING or PAUSED.";
        LOG_WARN("FSM", oss.str());
        return false;
    }

    m_state = MachineState::STOPPING;
    LOG_INFO("FSM", "[MachineFSM] State transition: " + machineStateToString(m_state) + " -> STOPPING.");

    m_hwManager.stopAll();

    m_state = MachineState::IDLE;
    LOG_INFO("FSM", "[MachineFSM] Safety shutdown complete. State transition: STOPPING -> IDLE.");
    if (m_eventHistory != nullptr) {
        m_eventHistory->addEvent(EventType::MACHINE_STOPPED, LogLevel::LOG_INFO_LEVEL, "Machine stopped and returned to IDLE state.");
    }
    return true;
}

void MachineFSM::emergencyStop() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    LOG_CRITICAL("FSM", "[MachineFSM] EMERGENCY STOP ASSERTED! Executing immediate hardware shutdown!");
    m_hwManager.stopAll();
    m_state = MachineState::EMERGENCY_STOP;
    LOG_CRITICAL("FSM", "[MachineFSM] State transition: -> EMERGENCY_STOP.");
    if (m_eventHistory != nullptr) {
        m_eventHistory->addEvent(EventType::EMERGENCY_STOP, LogLevel::LOG_CRITICAL_LEVEL, "EMERGENCY STOP ASSERTED! Hardware shut down immediately.");
    }
}

bool MachineFSM::reset() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    if (m_state != MachineState::ERROR && m_state != MachineState::EMERGENCY_STOP) {
        std::ostringstream oss;
        oss << "[MachineFSM] Invalid reset request from state " << machineStateToString(m_state) << ". Must be ERROR or EMERGENCY_STOP.";
        LOG_WARN("FSM", oss.str());
        return false;
    }

    if (!m_safetyController.isSafeToStart(m_hwManager)) {
        LOG_ERROR("FSM", "[MachineFSM] Reset REJECTED: Hardware safety interlock still reporting fault!");
        return false;
    }

    m_state = MachineState::IDLE;
    LOG_INFO("FSM", "[MachineFSM] Fault cleared. System reset cleanly. State transition: -> IDLE.");
    if (m_eventHistory != nullptr) {
        m_eventHistory->addEvent(EventType::MACHINE_RESET, LogLevel::LOG_INFO_LEVEL, "System fault cleared and reset to IDLE state.");
    }
    return true;
}

bool MachineFSM::triggerFault(const std::string& reason) {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    LOG_ERROR("FSM", "[MachineFSM] Operational fault triggered: " + reason);
    m_hwManager.stopAll();
    m_state = MachineState::ERROR;
    if (m_eventHistory != nullptr) {
        m_eventHistory->addEvent(EventType::HEALTH_FAULT, LogLevel::LOG_ERROR_LEVEL, "Operational fault triggered: " + reason);
    }
    return true;
}

} // namespace smartwaste::controller
