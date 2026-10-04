#include "FileEventRepository.hpp"
#include "../config/json.hpp"
#include "../logging/Logger.hpp"
#include "../core/OperationalEvent.hpp"

#include <fstream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace smartwaste::persistence {

using json = nlohmann::json;

static json eventToJson(const smartwaste::core::OperationalEvent& event) {
    json j;
    j["eventId"] = event.eventId;
    j["timestampMs"] = event.timestampMs;
    j["type"] = smartwaste::core::eventTypeToString(event.type);
    j["severity"] = smartwaste::logging::logLevelToString(event.severity);
    j["message"] = event.message;
    j["itemId"] = event.itemId;
    j["wasteType"] = smartwaste::core::wasteTypeToString(event.wasteType);
    j["gateId"] = event.gateId;
    return j;
}

static bool jsonToEvent(const json& j, smartwaste::core::OperationalEvent& event) {
    if (!j.is_object()) {
        return false;
    }
    try {
        if (j.contains("eventId") && j["eventId"].is_string()) {
            event.eventId = j["eventId"].get<std::string>();
        }
        if (j.contains("timestampMs") && j["timestampMs"].is_number()) {
            event.timestampMs = j["timestampMs"].get<std::int64_t>();
        }
        if (j.contains("type") && j["type"].is_string()) {
            event.type = smartwaste::core::stringToEventType(j["type"].get<std::string>());
        }
        if (j.contains("severity") && j["severity"].is_string()) {
            event.severity = smartwaste::logging::stringToLogLevel(j["severity"].get<std::string>());
        }
        if (j.contains("message") && j["message"].is_string()) {
            event.message = j["message"].get<std::string>();
        }
        if (j.contains("itemId") && j["itemId"].is_string()) {
            event.itemId = j["itemId"].get<std::string>();
        }
        if (j.contains("wasteType") && j["wasteType"].is_string()) {
            event.wasteType = smartwaste::core::stringToWasteType(j["wasteType"].get<std::string>());
        }
        if (j.contains("gateId") && j["gateId"].is_number()) {
            event.gateId = j["gateId"].get<int>();
        }
        return true;
    } catch (...) {
        return false;
    }
}

FileEventRepository::FileEventRepository(std::string filePath)
    : m_filePath(std::move(filePath)) {}

bool FileEventRepository::ensureDirectoryExists() const {
    std::size_t lastSlash = m_filePath.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return true;
    }
    std::string dir = m_filePath.substr(0, lastSlash);
    if (dir.empty()) {
        return true;
    }
#if defined(_WIN32) || defined(_WIN64)
    int res = _mkdir(dir.c_str());
#else
    int res = mkdir(dir.c_str(), 0755);
#endif
    (void)res;
    return true;
}

bool FileEventRepository::saveEvent(const smartwaste::core::OperationalEvent& event) {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    if (!ensureDirectoryExists()) {
        LOG_ERROR("FileEventRepository", "Failed to create directory for " + m_filePath);
        return false;
    }

    std::ofstream file(m_filePath, std::ios::out | std::ios::app);
    if (!file.is_open()) {
        LOG_ERROR("FileEventRepository", "Failed to open event persistence file for writing: " + m_filePath);
        return false;
    }

    json j = eventToJson(event);
    file << j.dump() << "\n";
    file.flush();
    return file.good();
}

bool FileEventRepository::saveEvents(const std::vector<smartwaste::core::OperationalEvent>& events) {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    if (!ensureDirectoryExists()) {
        LOG_ERROR("FileEventRepository", "Failed to create directory for " + m_filePath);
        return false;
    }

    std::ofstream file(m_filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("FileEventRepository", "Failed to open event persistence file for batch writing: " + m_filePath);
        return false;
    }

    for (const auto& event : events) {
        json j = eventToJson(event);
        file << j.dump() << "\n";
    }
    file.flush();
    return file.good();
}

std::vector<smartwaste::core::OperationalEvent> FileEventRepository::loadEvents() {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    std::vector<smartwaste::core::OperationalEvent> events;
    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        // Missing or unreadable file returns empty list safely
        return events;
    }

    std::string line;
    std::size_t lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        if (line.empty()) {
            continue;
        }

        // Non-throwing JSON parsing
        auto j = json::parse(line, nullptr, false);
        if (j.is_discarded()) {
            LOG_WARN("FileEventRepository", "Skipping malformed JSON record at line " + std::to_string(lineNum) + " in " + m_filePath);
            continue;
        }

        smartwaste::core::OperationalEvent evt;
        if (jsonToEvent(j, evt)) {
            events.push_back(evt);
        } else {
            LOG_WARN("FileEventRepository", "Skipping invalid event object at line " + std::to_string(lineNum) + " in " + m_filePath);
        }
    }
    return events;
}

bool FileEventRepository::clear() {
    smartwaste::hardware::ScopedLock lock(m_mutex);

    std::ofstream file(m_filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.close();
    return true;
}

} // namespace smartwaste::persistence
