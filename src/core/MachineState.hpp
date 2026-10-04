#ifndef SMARTWASTE_CORE_MACHINESTATE_HPP
#define SMARTWASTE_CORE_MACHINESTATE_HPP

#include <string>

namespace smartwaste::core {

enum class MachineState {
    IDLE,
    STARTING,
    RUNNING,
    PAUSED,
    STOPPING,
    ERROR,
    EMERGENCY_STOP
};

/**
 * @brief Converts a MachineState enum value to string.
 * @param state The MachineState enum value.
 * @return std::string Human-readable name of the machine state.
 */
std::string machineStateToString(MachineState state);

/**
 * @brief Safely parses a string into a MachineState enum value.
 * @param str String representation of machine state (case-insensitive).
 * @return MachineState Corresponding enum value, or MachineState::ERROR if unmapped.
 */
MachineState stringToMachineState(const std::string& str);

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_MACHINESTATE_HPP
