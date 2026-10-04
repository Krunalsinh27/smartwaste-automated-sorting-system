#ifndef SMARTWASTE_CORE_EVENTHISTORY_HPP
#define SMARTWASTE_CORE_EVENTHISTORY_HPP

#include "OperationalEvent.hpp"
#include "../hardware/HardwareLock.hpp"

#include <vector>
#include <string>
#include <cstddef>

namespace smartwaste::persistence {
class IEventRepository;
}

namespace smartwaste::core {

/**
 * @brief Thread-safe in-memory operational event history store.
 * Records, queries, and filters operational events occurring across the SmartWaste system.
 * Optionally integrates with an IEventRepository for persistence.
 */
class EventHistory {
public:
    explicit EventHistory(smartwaste::persistence::IEventRepository* repository = nullptr);
    ~EventHistory() = default;

    EventHistory(const EventHistory&) = delete;
    EventHistory& operator=(const EventHistory&) = delete;

    /**
     * @brief Sets or updates the persistence repository implementation.
     */
    void setRepository(smartwaste::persistence::IEventRepository* repository);

    /**
     * @brief Returns current active persistence repository pointer.
     */
    smartwaste::persistence::IEventRepository* getRepository() const noexcept { return m_repository; }

    /**
     * @brief Loads previously persisted events from repository into memory and restores sequence counter.
     * @return Number of events loaded.
     */
    std::size_t loadFromRepository();

    /**
     * @brief Adds a fully-formed OperationalEvent object into history (assigning sequential ID if missing).
     */
    OperationalEvent addEvent(OperationalEvent event);

    /**
     * @brief Helper to construct and add an OperationalEvent into history with auto-assigned ID and timestamp.
     */
    OperationalEvent addEvent(
        EventType type,
        smartwaste::logging::LogLevel severity,
        const std::string& message,
        const std::string& itemId = "",
        WasteType wasteType = WasteType::UNKNOWN,
        int gateId = 0
    );

    /**
     * @brief Returns total number of events recorded in history.
     */
    std::size_t getEventCount() const;

    /**
     * @brief Returns a copy of all recorded events in chronological order.
     */
    std::vector<OperationalEvent> getEvents() const;

    /**
     * @brief Returns up to count recent events (newest N items in chronological order).
     * @param count Number of recent events to retrieve.
     */
    std::vector<OperationalEvent> getRecentEvents(std::size_t count) const;

    /**
     * @brief Filters events matching a specific EventType.
     */
    std::vector<OperationalEvent> getEventsByType(EventType type) const;

    /**
     * @brief Filters events associated with a specific item ID.
     */
    std::vector<OperationalEvent> getEventsByItem(const std::string& itemId) const;

    /**
     * @brief Clears all stored events and resets ID counter to 1.
     */
    void clear();

private:
    std::string generateEventId();

    std::vector<OperationalEvent> m_events;
    std::size_t m_nextSequenceId{1};
    smartwaste::persistence::IEventRepository* m_repository{nullptr};
    mutable smartwaste::hardware::Mutex m_mutex;
};

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_EVENTHISTORY_HPP
