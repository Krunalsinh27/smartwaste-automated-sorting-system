#include "MachineState.hpp"
#include <algorithm>
#include <cctype>

namespace smartwaste::core {

std::string machineStateToString(MachineState state) {
    switch (state) {
        case MachineState::IDLE:           return "IDLE";
        case MachineState::STARTING:       return "STARTING";
        case MachineState::RUNNING:        return "RUNNING";
        case MachineState::PAUSED:         return "PAUSED";
        case MachineState::STOPPING:       return "STOPPING";
        case MachineState::ERROR:          return "ERROR";
        case MachineState::EMERGENCY_STOP: return "EMERGENCY_STOP";
        default:                           return "UNKNOWN";
    }
}

MachineState stringToMachineState(const std::string& str) {
    std::string upperStr;
    upperStr.reserve(str.size());
    for (char ch : str) {
        upperStr.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }

    if (upperStr == "IDLE")           return MachineState::IDLE;
    if (upperStr == "STARTING")       return MachineState::STARTING;
    if (upperStr == "RUNNING")        return MachineState::RUNNING;
    if (upperStr == "PAUSED")         return MachineState::PAUSED;
    if (upperStr == "STOPPING")       return MachineState::STOPPING;
    if (upperStr == "ERROR")          return MachineState::ERROR;
    if (upperStr == "EMERGENCY_STOP") return MachineState::EMERGENCY_STOP;

    return MachineState::ERROR;
}

} // namespace smartwaste::core
