#include "OperationalEvent.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace smartwaste::core {

std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::ITEM_DETECTED:    return "ITEM_DETECTED";
        case EventType::ITEM_CLASSIFIED:  return "ITEM_CLASSIFIED";
        case EventType::ITEM_SORTED:      return "ITEM_SORTED";
        case EventType::ITEM_REJECTED:    return "ITEM_REJECTED";
        case EventType::GATE_ACTUATED:    return "GATE_ACTUATED";
        case EventType::GATE_JAMMED:      return "GATE_JAMMED";
        case EventType::GATE_UNAVAILABLE: return "GATE_UNAVAILABLE";
        case EventType::HEALTH_WARNING:   return "HEALTH_WARNING";
        case EventType::HEALTH_FAULT:     return "HEALTH_FAULT";
        case EventType::MACHINE_STARTED:  return "MACHINE_STARTED";
        case EventType::MACHINE_STOPPED:  return "MACHINE_STOPPED";
        case EventType::EMERGENCY_STOP:   return "EMERGENCY_STOP";
        case EventType::MACHINE_RESET:    return "MACHINE_RESET";
        case EventType::UNKNOWN:
        default:                          return "UNKNOWN";
    }
}

EventType stringToEventType(const std::string& str) {
    std::string upperStr = str;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });

    if (upperStr == "ITEM_DETECTED")    return EventType::ITEM_DETECTED;
    if (upperStr == "ITEM_CLASSIFIED")  return EventType::ITEM_CLASSIFIED;
    if (upperStr == "ITEM_SORTED")      return EventType::ITEM_SORTED;
    if (upperStr == "ITEM_REJECTED")    return EventType::ITEM_REJECTED;
    if (upperStr == "GATE_ACTUATED")    return EventType::GATE_ACTUATED;
    if (upperStr == "GATE_JAMMED")      return EventType::GATE_JAMMED;
    if (upperStr == "GATE_UNAVAILABLE") return EventType::GATE_UNAVAILABLE;
    if (upperStr == "HEALTH_WARNING")   return EventType::HEALTH_WARNING;
    if (upperStr == "HEALTH_FAULT")     return EventType::HEALTH_FAULT;
    if (upperStr == "MACHINE_STARTED")  return EventType::MACHINE_STARTED;
    if (upperStr == "MACHINE_STOPPED")  return EventType::MACHINE_STOPPED;
    if (upperStr == "EMERGENCY_STOP")   return EventType::EMERGENCY_STOP;
    if (upperStr == "MACHINE_RESET")    return EventType::MACHINE_RESET;

    return EventType::UNKNOWN;
}

std::string OperationalEvent::toString() const {
    std::ostringstream oss;
    oss << "[" << eventId << "] "
        << "Type: " << eventTypeToString(type)
        << " | Severity: " << smartwaste::logging::logLevelToString(severity)
        << " | Details: " << message;

    if (!itemId.empty()) {
        oss << " | Item: " << itemId;
    }
    if (wasteType != WasteType::UNKNOWN) {
        oss << " | WasteType: " << wasteTypeToString(wasteType);
    }
    if (gateId > 0) {
        oss << " | Gate: #" << gateId;
    }

    return oss.str();
}

} // namespace smartwaste::core
