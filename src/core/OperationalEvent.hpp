#ifndef SMARTWASTE_CORE_OPERATIONALEVENT_HPP
#define SMARTWASTE_CORE_OPERATIONALEVENT_HPP

#include "WasteType.hpp"
#include "../logging/Logger.hpp"

#include <string>
#include <cstdint>

namespace smartwaste::core {

/**
 * @brief Enum representing operational event categories within the SmartWaste system.
 */
enum class EventType {
    ITEM_DETECTED,
    ITEM_CLASSIFIED,
    ITEM_SORTED,
    ITEM_REJECTED,
    GATE_ACTUATED,
    GATE_JAMMED,
    GATE_UNAVAILABLE,
    HEALTH_WARNING,
    HEALTH_FAULT,
    MACHINE_STARTED,
    MACHINE_STOPPED,
    EMERGENCY_STOP,
    MACHINE_RESET,
    UNKNOWN
};

/**
 * @brief Converts EventType enum to string representation.
 */
std::string eventTypeToString(EventType type);

/**
 * @brief Parses string representation into EventType enum safely.
 */
EventType stringToEventType(const std::string& str);

/**
 * @brief Domain model representing an operational system event in memory.
 */
struct OperationalEvent {
    std::string eventId;
    std::int64_t timestampMs{0};
    EventType type{EventType::UNKNOWN};
    smartwaste::logging::LogLevel severity{smartwaste::logging::LogLevel::LOG_INFO_LEVEL};
    std::string message;
    std::string itemId;
    WasteType wasteType{WasteType::UNKNOWN};
    int gateId{0};

    std::string toString() const;
};

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_OPERATIONALEVENT_HPP
