#include "SmartWasteEngine.hpp"
#include <sstream>
#include <exception>

namespace smartwaste::engine {

std::string EngineSnapshot::toString() const {
    std::ostringstream oss;
    oss << "=== SmartWaste Engine Snapshot ===\n"
        << "State: " << smartwaste::core::machineStateToString(systemStatus.state) << "\n"
        << "Conveyor: " << (systemStatus.isConveyorRunning ? "RUNNING" : "STOPPED") << " (" << telemetry.conveyorSpeed << " m/s)\n"
        << "Motor: " << telemetry.motorRPM << " RPM\n"
        << "Health: " << smartwaste::hardware::healthStatusToString(health.status) << " (" << health.message << ")\n"
        << "Items Processed: " << statistics.totalItems << " (Success: " << statistics.successfulItems
        << ", Rejected: " << statistics.rejectedItems << ", Gate Unavail: " << statistics.unavailableGateItems
        << ", Failed: " << statistics.failedItems << ")\n"
        << "Total Persisted Events: " << totalEvents << " [" << persistencePath << "]";
    return oss.str();
}

SmartWasteEngine::SmartWasteEngine()
    : m_hardwareManager(m_conveyor, m_motor),
      m_telemetryAggregator(m_conveyor, m_motor, m_sensorInfeed, m_gatePlastic),
      m_safetyController(m_healthMonitor),
      m_sortingController(m_configManager),
      m_eventRepository("data/events.jsonl"),
      m_eventHistory(&m_eventRepository),
      m_machineFSM(m_hardwareManager, m_safetyController, &m_eventHistory)
{
    registerHardwareComponents();
}

SmartWasteEngine::SmartWasteEngine(std::string eventStoragePath)
    : m_hardwareManager(m_conveyor, m_motor),
      m_telemetryAggregator(m_conveyor, m_motor, m_sensorInfeed, m_gatePlastic),
      m_safetyController(m_healthMonitor),
      m_sortingController(m_configManager),
      m_eventRepository(std::move(eventStoragePath)),
      m_eventHistory(&m_eventRepository),
      m_machineFSM(m_hardwareManager, m_safetyController, &m_eventHistory)
{
    registerHardwareComponents();
}

void SmartWasteEngine::registerHardwareComponents() {
    m_hardwareManager.addSensor(m_sensorInfeed);
    m_hardwareManager.addGate(m_gatePlastic);
    m_hardwareManager.addGate(m_gateMetal);
    m_hardwareManager.addGate(m_gatePaper);
    m_hardwareManager.addGate(m_gateGlass);
    m_hardwareManager.addGate(m_gateOther);
}

bool SmartWasteEngine::initialize(const std::string& configFilePath) {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);

    try {
        m_configManager.loadFromFile(configFilePath);
    } catch (const std::exception& e) {
        LOG_WARN("SmartWasteEngine", "Configuration load warning for '" + configFilePath + "': " + e.what() + ". Using defaults.");
    }

    // Load historical events from persistence repository
    m_eventHistory.loadFromRepository();

    m_isInitialized = true;
    LOG_INFO("SmartWasteEngine", "Engine initialized cleanly. Restored operational events: " + std::to_string(m_eventHistory.getEventCount()));
    return true;
}

smartwaste::core::MachineState SmartWasteEngine::getMachineState() const {
    return m_machineFSM.getCurrentState();
}

std::string SmartWasteEngine::getMachineStateString() const {
    return m_machineFSM.getStateString();
}

bool SmartWasteEngine::startMachine() {
    return m_machineFSM.start();
}

bool SmartWasteEngine::pauseMachine() {
    return m_machineFSM.pause();
}

bool SmartWasteEngine::resumeMachine() {
    return m_machineFSM.resume();
}

bool SmartWasteEngine::stopMachine() {
    return m_machineFSM.stop();
}

void SmartWasteEngine::emergencyStop() {
    m_machineFSM.emergencyStop();
}

bool SmartWasteEngine::resetMachine() {
    return m_machineFSM.reset();
}

smartwaste::core::SortingResult SmartWasteEngine::processWasteItem(const smartwaste::core::WasteItem& item) {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);

    if (m_machineFSM.getCurrentState() != smartwaste::core::MachineState::RUNNING) {
        smartwaste::core::SortingResult result;
        result.type = item.type;
        result.targetGate = 0;
        result.confidence = item.confidence;
        result.status = "REJECTED";
        result.message = "Machine must be in RUNNING state to process waste (Current state: " +
                         m_machineFSM.getStateString() + ").";
        LOG_WARN("SmartWasteEngine", result.message);
        return result;
    }

    return m_simulationRunner.processItem(
        item,
        m_sortingController,
        m_hardwareManager,
        &m_telemetryAggregator,
        &m_healthMonitor,
        &m_eventHistory
    );
}

smartwaste::hardware::SimulationStatistics SmartWasteEngine::runSimulation(const std::vector<smartwaste::core::WasteItem>& items) {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);
    return m_simulationRunner.runSimulation(
        items,
        m_sortingController,
        m_hardwareManager,
        &m_telemetryAggregator,
        &m_healthMonitor,
        &m_eventHistory
    );
}

smartwaste::core::SystemStatus SmartWasteEngine::getSystemStatus() const {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);

    smartwaste::core::SystemStatus status;
    status.state = m_machineFSM.getCurrentState();
    status.isCameraConnected = m_isInitialized;
    status.isConveyorRunning = m_conveyor.isRunning();
    status.isMotorNormal = (m_motor.getRPM() > 0.0 || !m_conveyor.isRunning());
    status.isEmergencyStopActive = (status.state == smartwaste::core::MachineState::EMERGENCY_STOP);
    status.currentDetectedWaste = smartwaste::core::WasteType::UNKNOWN;
    status.totalProcessedItems = m_simulationRunner.getStatistics().totalItems;
    return status;
}

smartwaste::hardware::HardwareTelemetry SmartWasteEngine::getTelemetry() const {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);
    return m_telemetryAggregator.read();
}

smartwaste::hardware::HardwareHealth SmartWasteEngine::getHealthStatus() const {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);
    auto telem = m_telemetryAggregator.read();
    return m_healthMonitor.evaluate(telem);
}

smartwaste::hardware::SimulationStatistics SmartWasteEngine::getSimulationStatistics() const {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);
    return m_simulationRunner.getStatistics();
}

EngineSnapshot SmartWasteEngine::getSnapshot() const {
    smartwaste::hardware::ScopedLock lock(m_engineMutex);

    EngineSnapshot snap;
    snap.systemStatus = getSystemStatus();
    snap.telemetry = m_telemetryAggregator.read();
    snap.health = m_healthMonitor.evaluate(snap.telemetry);
    snap.statistics = m_simulationRunner.getStatistics();
    snap.totalEvents = m_eventHistory.getEventCount();
    snap.persistencePath = m_eventRepository.getFilePath();
    snap.isInitialized = m_isInitialized;
    return snap;
}

std::size_t SmartWasteEngine::getEventCount() const {
    return m_eventHistory.getEventCount();
}

std::vector<smartwaste::core::OperationalEvent> SmartWasteEngine::getRecentEvents(std::size_t count) const {
    return m_eventHistory.getRecentEvents(count);
}

std::vector<smartwaste::core::OperationalEvent> SmartWasteEngine::getEventsByType(smartwaste::core::EventType type) const {
    return m_eventHistory.getEventsByType(type);
}

std::vector<smartwaste::core::OperationalEvent> SmartWasteEngine::getEventsByItem(const std::string& itemId) const {
    return m_eventHistory.getEventsByItem(itemId);
}

std::vector<smartwaste::core::OperationalEvent> SmartWasteEngine::getEvents() const {
    return m_eventHistory.getEvents();
}

void SmartWasteEngine::clearEventHistory() {
    m_eventHistory.clear();
}

const smartwaste::config::SystemConfig& SmartWasteEngine::getConfig() const {
    return m_configManager.getConfig();
}

smartwaste::hardware::SimulatedGate* SmartWasteEngine::getGate(std::size_t index) {
    if (index == 0) return &m_gatePlastic;
    if (index == 1) return &m_gateMetal;
    if (index == 2) return &m_gatePaper;
    if (index == 3) return &m_gateGlass;
    if (index == 4) return &m_gateOther;
    return nullptr;
}

} // namespace smartwaste::engine
