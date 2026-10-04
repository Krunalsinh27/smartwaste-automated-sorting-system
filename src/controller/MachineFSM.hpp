#ifndef SMARTWASTE_CONTROLLER_MACHINEFSM_HPP
#define SMARTWASTE_CONTROLLER_MACHINEFSM_HPP

#include "../core/MachineState.hpp"
#include "../hardware/HardwareManager.hpp"
#include "../hardware/HardwareLock.hpp"
#include "SafetyController.hpp"
#include <string>

namespace smartwaste::core {
    class EventHistory;
}

namespace smartwaste::controller {

/**
 * @brief Finite State Machine controlling the operational lifecycle and safety transitions
 * of the SmartWaste automated sorting system.
 */
class MachineFSM {
public:
    MachineFSM(
        smartwaste::hardware::HardwareManager& hwManager,
        const SafetyController& safetyController,
        smartwaste::core::EventHistory* eventHistory = nullptr
    );

    void setEventHistory(smartwaste::core::EventHistory* eventHistory) noexcept;

    ~MachineFSM() = default;

    MachineFSM(const MachineFSM&) = delete;
    MachineFSM& operator=(const MachineFSM&) = delete;

    // FSM State Queries
    smartwaste::core::MachineState getCurrentState() const;
    std::string getStateString() const;

    // Lifecycle Commands
    bool start();
    bool pause();
    bool resume();
    bool stop();
    void emergencyStop();
    bool reset();
    bool triggerFault(const std::string& reason);

private:
    bool isValidTransition(smartwaste::core::MachineState from, smartwaste::core::MachineState to) const;

    smartwaste::core::MachineState m_state{smartwaste::core::MachineState::IDLE};
    smartwaste::hardware::HardwareManager& m_hwManager;
    const SafetyController& m_safetyController;
    smartwaste::core::EventHistory* m_eventHistory{nullptr};

    mutable smartwaste::hardware::Mutex m_mutex;
};

} // namespace smartwaste::controller

#endif // SMARTWASTE_CONTROLLER_MACHINEFSM_HPP
