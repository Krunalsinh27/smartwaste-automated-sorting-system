#ifndef SMARTWASTE_ENGINE_SMARTWASTEENGINE_HPP
#define SMARTWASTE_ENGINE_SMARTWASTEENGINE_HPP

#include "../core/WasteType.hpp"
#include "../core/MachineState.hpp"
#include "../core/SystemStatus.hpp"
#include "../core/WasteItem.hpp"
#include "../core/SortingResult.hpp"
#include "../core/OperationalEvent.hpp"
#include "../core/EventHistory.hpp"

#include "../config/ConfigManager.hpp"
#include "../logging/Logger.hpp"

#include "../hardware/SimulatedConveyor.hpp"
#include "../hardware/SimulatedMotor.hpp"
#include "../hardware/SimulatedSensor.hpp"
#include "../hardware/SimulatedGate.hpp"
#include "../hardware/HardwareManager.hpp"
#include "../hardware/HardwareTelemetryAggregator.hpp"
#include "../hardware/SystemHealthMonitor.hpp"
#include "../hardware/SimulationRunner.hpp"
#include "../hardware/HardwareLock.hpp"

#include "../controller/SafetyController.hpp"
#include "../controller/MachineFSM.hpp"
#include "../controller/SortingController.hpp"

#include "../persistence/FileEventRepository.hpp"

#include <string>
#include <vector>
#include <memory>
#include <cstddef>

namespace smartwaste::engine {

/**
 * @brief Unified snapshot containing complete system engine state for observability and future API layers.
 */
struct EngineSnapshot {
    smartwaste::core::SystemStatus systemStatus;
    smartwaste::hardware::HardwareTelemetry telemetry;
    smartwaste::hardware::HardwareHealth health;
    smartwaste::hardware::SimulationStatistics statistics;
    std::size_t totalEvents{0};
    std::string persistencePath;
    bool isInitialized{false};

    std::string toString() const;
};

/**
 * @brief High-level facade service layer coordinating all SmartWaste subsystems.
 * Encapsulates domain logic, hardware coordination, state machine lifecycle,
 * sorting decisions, telemetry aggregation, health monitoring, simulation execution,
 * and event persistence.
 */
class SmartWasteEngine {
public:
    SmartWasteEngine();
    explicit SmartWasteEngine(std::string eventStoragePath);
    ~SmartWasteEngine() = default;

    SmartWasteEngine(const SmartWasteEngine&) = delete;
    SmartWasteEngine& operator=(const SmartWasteEngine&) = delete;

    /**
     * @brief Initializes system configuration and prepares hardware & persistence layers.
     * @param configFilePath Path to system configuration JSON file.
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize(const std::string& configFilePath = "config/config.json");

    // Machine Lifecycle Commands
    smartwaste::core::MachineState getMachineState() const;
    std::string getMachineStateString() const;

    bool startMachine();
    bool pauseMachine();
    bool resumeMachine();
    bool stopMachine();
    void emergencyStop();
    bool resetMachine();

    // Sorting & Simulation Operations
    smartwaste::core::SortingResult processWasteItem(const smartwaste::core::WasteItem& item);
    smartwaste::hardware::SimulationStatistics runSimulation(const std::vector<smartwaste::core::WasteItem>& items);

    // Observability & Telemetry Snapshots
    smartwaste::core::SystemStatus getSystemStatus() const;
    smartwaste::hardware::HardwareTelemetry getTelemetry() const;
    smartwaste::hardware::HardwareHealth getHealthStatus() const;
    smartwaste::hardware::SimulationStatistics getSimulationStatistics() const;
    EngineSnapshot getSnapshot() const;

    // Operational Event History & Persistence
    std::size_t getEventCount() const;
    std::vector<smartwaste::core::OperationalEvent> getRecentEvents(std::size_t count = 10) const;
    std::vector<smartwaste::core::OperationalEvent> getEventsByType(smartwaste::core::EventType type) const;
    std::vector<smartwaste::core::OperationalEvent> getEventsByItem(const std::string& itemId) const;
    std::vector<smartwaste::core::OperationalEvent> getEvents() const;
    void clearEventHistory();

    // Configuration & Component Accessors
    const smartwaste::config::SystemConfig& getConfig() const;
    bool isInitialized() const noexcept { return m_isInitialized; }

    smartwaste::hardware::HardwareManager& getHardwareManager() noexcept { return m_hardwareManager; }
    smartwaste::hardware::SimulatedConveyor& getConveyor() noexcept { return m_conveyor; }
    smartwaste::hardware::SimulatedMotor& getMotor() noexcept { return m_motor; }
    smartwaste::hardware::SimulatedGate* getGate(std::size_t index);

private:
    void registerHardwareComponents();

    bool m_isInitialized{false};

    // Configuration
    smartwaste::config::ConfigManager m_configManager;

    // Simulated Physical Hardware Components
    smartwaste::hardware::SimulatedConveyor m_conveyor{4.0};
    smartwaste::hardware::SimulatedMotor m_motor{1800.0};
    smartwaste::hardware::SimulatedSensor m_sensorInfeed{1.0};
    smartwaste::hardware::SimulatedGate m_gatePlastic{1};
    smartwaste::hardware::SimulatedGate m_gateMetal{2};
    smartwaste::hardware::SimulatedGate m_gatePaper{3};
    smartwaste::hardware::SimulatedGate m_gateGlass{4};
    smartwaste::hardware::SimulatedGate m_gateOther{5};

    // Hardware Manager & Telemetry / Health
    smartwaste::hardware::HardwareManager m_hardwareManager;
    smartwaste::hardware::HardwareTelemetryAggregator m_telemetryAggregator;
    smartwaste::hardware::SystemHealthMonitor m_healthMonitor;

    // Controllers & Subsystems
    smartwaste::controller::SafetyController m_safetyController;
    smartwaste::controller::SortingController m_sortingController;
    smartwaste::persistence::FileEventRepository m_eventRepository;
    smartwaste::core::EventHistory m_eventHistory;
    smartwaste::controller::MachineFSM m_machineFSM;

    // Simulation Execution Engine
    smartwaste::hardware::SimulationRunner m_simulationRunner;

    mutable smartwaste::hardware::Mutex m_engineMutex;
};

} // namespace smartwaste::engine

#endif // SMARTWASTE_ENGINE_SMARTWASTEENGINE_HPP
