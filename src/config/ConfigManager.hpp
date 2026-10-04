#ifndef SMARTWASTE_CONFIG_CONFIGMANAGER_HPP
#define SMARTWASTE_CONFIG_CONFIGMANAGER_HPP

#include <string>
#include <stdexcept>

namespace smartwaste::config {

/**
 * @brief Custom exception for configuration loading and validation failures.
 */
class ConfigException : public std::runtime_error {
public:
    explicit ConfigException(const std::string& message)
        : std::runtime_error("[ConfigException] " + message) {}
};

struct MachineConfig {
    std::string name;
    double maxConveyorSpeed{0.0};
    int maxGates{0};
    int autoStopTimeoutSec{0};
};

struct CameraConfig {
    int deviceId{0};
    int frameWidth{0};
    int frameHeight{0};
    int fps{0};
    double minConfidenceThreshold{0.0};
};

struct SortingConfig {
    int plasticGate{0};
    int metalGate{0};
    int paperGate{0};
    int glassGate{0};
    int defaultOtherGate{0};
    double gateActuationDurationMs{0.0};
};

struct SystemConfig {
    MachineConfig machine;
    CameraConfig camera;
    SortingConfig sorting;
    std::string eventStoragePath{"data/events.jsonl"};
    std::string databasePath{"data/smartwaste.db"};
};

/**
 * @brief Thread-safe configuration manager responsible for loading, parsing,
 * and validating JSON runtime settings.
 */
class ConfigManager {
public:
    ConfigManager() = default;

    /**
     * @brief Loads and validates system configuration from a JSON file.
     * @param filePath Path to the JSON configuration file.
     * @throws ConfigException if file access, parsing, or validation fails.
     */
    void loadFromFile(const std::string& filePath);

    const SystemConfig& getConfig() const noexcept { return m_config; }
    const MachineConfig& getMachineConfig() const noexcept { return m_config.machine; }
    const CameraConfig& getCameraConfig() const noexcept { return m_config.camera; }
    const SortingConfig& getSortingConfig() const noexcept { return m_config.sorting; }
    const std::string& getEventStoragePath() const noexcept { return m_config.eventStoragePath; }
    const std::string& getDatabasePath() const noexcept { return m_config.databasePath; }
    bool isLoaded() const noexcept { return m_isLoaded; }


private:
    void validate() const;

    SystemConfig m_config;
    bool m_isLoaded{false};
};

} // namespace smartwaste::config

#endif // SMARTWASTE_CONFIG_CONFIGMANAGER_HPP
