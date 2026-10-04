#ifndef SMARTWASTE_PERSISTENCE_IEVENTREPOSITORY_HPP
#define SMARTWASTE_PERSISTENCE_IEVENTREPOSITORY_HPP

#include "../core/OperationalEvent.hpp"
#include <vector>

namespace smartwaste::persistence {

/**
 * @brief Abstract persistence repository interface for operational events.
 * Provides storage and retrieval operations without coupling core domain logic
 * to specific storage implementations.
 */
class IEventRepository {
public:
    virtual ~IEventRepository() = default;

    /**
     * @brief Persists a single operational event.
     * @param event The event to save.
     * @return true if successful, false otherwise.
     */
    virtual bool saveEvent(const smartwaste::core::OperationalEvent& event) = 0;

    /**
     * @brief Persists a batch of operational events (overwrites existing store).
     * @param events Vector of events to save.
     * @return true if successful, false otherwise.
     */
    virtual bool saveEvents(const std::vector<smartwaste::core::OperationalEvent>& events) = 0;

    /**
     * @brief Loads all persisted operational events into memory.
     * @return Vector of events in chronological order.
     */
    virtual std::vector<smartwaste::core::OperationalEvent> loadEvents() = 0;

    /**
     * @brief Clears all persisted events from storage.
     * @return true if successful, false otherwise.
     */
    virtual bool clear() = 0;
};

} // namespace smartwaste::persistence

#endif // SMARTWASTE_PERSISTENCE_IEVENTREPOSITORY_HPP
