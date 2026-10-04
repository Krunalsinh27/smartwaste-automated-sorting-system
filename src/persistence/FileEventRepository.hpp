#ifndef SMARTWASTE_PERSISTENCE_FILEEVENTREPOSITORY_HPP
#define SMARTWASTE_PERSISTENCE_FILEEVENTREPOSITORY_HPP

#include "IEventRepository.hpp"
#include "../hardware/HardwareLock.hpp"
#include <string>

namespace smartwaste::persistence {

/**
 * @brief File-based operational event repository using JSON Lines (NDJSON) format.
 * Thread-safe implementation handling persistence, append-only writes, directory creation,
 * and malformed line recovery.
 */
class FileEventRepository : public IEventRepository {
public:
    explicit FileEventRepository(std::string filePath = "data/events.jsonl");
    ~FileEventRepository() override = default;

    FileEventRepository(const FileEventRepository&) = delete;
    FileEventRepository& operator=(const FileEventRepository&) = delete;

    bool saveEvent(const smartwaste::core::OperationalEvent& event) override;
    bool saveEvents(const std::vector<smartwaste::core::OperationalEvent>& events) override;
    std::vector<smartwaste::core::OperationalEvent> loadEvents() override;
    bool clear() override;

    const std::string& getFilePath() const noexcept { return m_filePath; }

private:
    bool ensureDirectoryExists() const;

    std::string m_filePath;
    mutable smartwaste::hardware::Mutex m_mutex;
};

} // namespace smartwaste::persistence

#endif // SMARTWASTE_PERSISTENCE_FILEEVENTREPOSITORY_HPP
