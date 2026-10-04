#include "EventHistory.hpp"
#include "../persistence/IEventRepository.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace smartwaste::core {

EventHistory::EventHistory(smartwaste::persistence::IEventRepository* repository)
    : m_repository(repository) {}

void EventHistory::setRepository(smartwaste::persistence::IEventRepository* repository) {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    m_repository = repository;
}

std::size_t EventHistory::loadFromRepository() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    if (!m_repository) {
        return 0;
    }

    auto loadedEvents = m_repository->loadEvents();
    m_events = loadedEvents;

    std::size_t maxSeq = 0;
    for (const auto& evt : m_events) {
        if (evt.eventId.rfind("EVT-", 0) == 0 && evt.eventId.length() > 4) {
            try {
                std::size_t seq = std::stoul(evt.eventId.substr(4));
                if (seq > maxSeq) {
                    maxSeq = seq;
                }
            } catch (...) {
                // Ignore parsing failure for non-numeric sequence formats
            }
        }
    }

    if (maxSeq > 0) {
        m_nextSequenceId = maxSeq + 1;
    } else if (m_events.empty()) {
        m_nextSequenceId = 1;
    } else {
        m_nextSequenceId = m_events.size() + 1;
    }

    return m_events.size();
}

std::string EventHistory::generateEventId() {
    std::ostringstream oss;
    oss << "EVT-" << std::setw(6) << std::setfill('0') << m_nextSequenceId++;
    return oss.str();
}

OperationalEvent EventHistory::addEvent(OperationalEvent event) {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    if (event.eventId.empty()) {
        event.eventId = generateEventId();
    }
    if (event.timestampMs <= 0) {
        event.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    m_events.push_back(event);

    if (m_repository) {
        m_repository->saveEvent(event);
    }

    return event;
}

OperationalEvent EventHistory::addEvent(
    EventType type,
    smartwaste::logging::LogLevel severity,
    const std::string& message,
    const std::string& itemId,
    WasteType wasteType,
    int gateId
) {
    OperationalEvent event;
    event.type = type;
    event.severity = severity;
    event.message = message;
    event.itemId = itemId;
    event.wasteType = wasteType;
    event.gateId = gateId;

    return addEvent(event);
}

std::size_t EventHistory::getEventCount() const {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    return m_events.size();
}

std::vector<OperationalEvent> EventHistory::getEvents() const {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    return m_events;
}

std::vector<OperationalEvent> EventHistory::getRecentEvents(std::size_t count) const {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    if (count == 0 || m_events.empty()) {
        return {};
    }

    if (count >= m_events.size()) {
        return m_events;
    }

    // Return the newest count items in chronological order
    std::vector<OperationalEvent> recent;
    recent.reserve(count);
    std::size_t startIdx = m_events.size() - count;
    for (std::size_t i = startIdx; i < m_events.size(); ++i) {
        recent.push_back(m_events[i]);
    }
    return recent;
}

std::vector<OperationalEvent> EventHistory::getEventsByType(EventType type) const {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    std::vector<OperationalEvent> result;
    for (const auto& evt : m_events) {
        if (evt.type == type) {
            result.push_back(evt);
        }
    }
    return result;
}

std::vector<OperationalEvent> EventHistory::getEventsByItem(const std::string& itemId) const {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    std::vector<OperationalEvent> result;
    if (itemId.empty()) {
        return result;
    }

    for (const auto& evt : m_events) {
        if (evt.itemId == itemId) {
            result.push_back(evt);
        }
    }
    return result;
}

void EventHistory::clear() {
    smartwaste::hardware::ScopedLock lock(m_mutex);
    m_events.clear();
    m_nextSequenceId = 1;
    if (m_repository) {
        m_repository->clear();
    }
}

} // namespace smartwaste::core
