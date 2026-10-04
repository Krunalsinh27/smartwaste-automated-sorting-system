#include "ConfigManager.hpp"
#include "json.hpp"
#include <fstream>
#include <sstream>

namespace smartwaste::config {

using json = nlohmann::json;

void ConfigManager::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw ConfigException("Could not open configuration file: " + filePath);
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        throw ConfigException("JSON syntax parse error in file " + filePath + ": " + e.what());
    }

    try {
        // Parse Machine section
        if (!j.contains("machine")) {
            throw ConfigException("Missing required configuration section: 'machine'");
        }
        const auto& m = j["machine"];
        m_config.machine.name = m.at("name").get<std::string>();
        m_config.machine.maxConveyorSpeed = m.at("maxConveyorSpeed").get<double>();
        m_config.machine.maxGates = m.at("maxGates").get<int>();
        m_config.machine.autoStopTimeoutSec = m.at("autoStopTimeoutSec").get<int>();

        // Parse Camera section
        if (!j.contains("camera")) {
            throw ConfigException("Missing required configuration section: 'camera'");
        }
        const auto& c = j["camera"];
        m_config.camera.deviceId = c.at("deviceId").get<int>();
        m_config.camera.frameWidth = c.at("frameWidth").get<int>();
        m_config.camera.frameHeight = c.at("frameHeight").get<int>();
        m_config.camera.fps = c.at("fps").get<int>();
        m_config.camera.minConfidenceThreshold = c.at("minConfidenceThreshold").get<double>();

        // Parse Sorting section
        if (!j.contains("sorting")) {
            throw ConfigException("Missing required configuration section: 'sorting'");
        }
        const auto& s = j["sorting"];
        m_config.sorting.plasticGate = s.at("plasticGate").get<int>();
        m_config.sorting.metalGate = s.at("metalGate").get<int>();
        m_config.sorting.paperGate = s.at("paperGate").get<int>();
        m_config.sorting.glassGate = s.at("glassGate").get<int>();
        m_config.sorting.defaultOtherGate = s.at("defaultOtherGate").get<int>();
        m_config.sorting.gateActuationDurationMs = s.at("gateActuationDurationMs").get<double>();

        // Parse optional persistence section
        if (j.contains("eventStoragePath") && j["eventStoragePath"].is_string()) {
            m_config.eventStoragePath = j["eventStoragePath"].get<std::string>();
        }
        if (j.contains("databasePath") && j["databasePath"].is_string()) {
            m_config.databasePath = j["databasePath"].get<std::string>();
        }


    } catch (const json::out_of_range& e) {
        throw ConfigException("Missing required field inside configuration section: " + std::string(e.what()));
    } catch (const json::type_error& e) {
        throw ConfigException("Type mismatch in configuration file: " + std::string(e.what()));
    }

    validate();
    m_isLoaded = true;
}

void ConfigManager::validate() const {
    if (m_config.machine.maxConveyorSpeed <= 0.0) {
        throw ConfigException("Validation failed: maxConveyorSpeed must be positive.");
    }
    if (m_config.machine.maxGates <= 0) {
        throw ConfigException("Validation failed: maxGates must be greater than zero.");
    }
    if (m_config.camera.frameWidth <= 0 || m_config.camera.frameHeight <= 0) {
        throw ConfigException("Validation failed: camera frame dimensions must be positive integers.");
    }
    if (m_config.camera.fps <= 0) {
        throw ConfigException("Validation failed: camera FPS must be positive.");
    }
    if (m_config.camera.minConfidenceThreshold < 0.0 || m_config.camera.minConfidenceThreshold > 1.0) {
        throw ConfigException("Validation failed: minConfidenceThreshold must be between 0.0 and 1.0.");
    }
    if (m_config.sorting.gateActuationDurationMs <= 0.0) {
        throw ConfigException("Validation failed: gateActuationDurationMs must be positive.");
    }
}

} // namespace smartwaste::config
