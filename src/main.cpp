#include "core/WasteType.hpp"
#include "core/MachineState.hpp"
#include "core/SystemStatus.hpp"
#include "core/WasteItem.hpp"
#include "core/SortingResult.hpp"
#include "core/OperationalEvent.hpp"
#include "core/EventHistory.hpp"
#include "config/ConfigManager.hpp"
#include "logging/Logger.hpp"

#include "hal/IConveyor.hpp"
#include "hal/IMotor.hpp"
#include "hal/ISensor.hpp"
#include "hal/IGate.hpp"

#include "hardware/SimulatedConveyor.hpp"
#include "hardware/SimulatedMotor.hpp"
#include "hardware/SimulatedSensor.hpp"
#include "hardware/SimulatedGate.hpp"
#include "hardware/HardwareTelemetry.hpp"
#include "hardware/HardwareTelemetryAggregator.hpp"
#include "hardware/SimulationRunner.hpp"
#include "hardware/HardwareManager.hpp"
#include "hardware/SystemHealthMonitor.hpp"

#include "controller/SafetyController.hpp"
#include "controller/MachineFSM.hpp"
#include "controller/SortingController.hpp"

#include "persistence/IEventRepository.hpp"
#include "persistence/FileEventRepository.hpp"
#include "database/SQLiteDatabase.hpp"

#include "engine/SmartWasteEngine.hpp"

#include "api/HttpServer.hpp"
#include "api/controllers/MachineController.hpp"

#include <iostream>
#include <chrono>
#include <memory>

using namespace smartwaste::core;
using namespace smartwaste::config;
using namespace smartwaste::logging;
using namespace smartwaste::hal;
using namespace smartwaste::hardware;
using namespace smartwaste::controller;
using namespace smartwaste::persistence;
using namespace smartwaste::engine;
using namespace smartwaste::api;
using namespace smartwaste::api::controllers;

int main() {
    // 1. Initialize Logger
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(LogLevel::LOG_DEBUG_LEVEL);
    bool logFileOk = logger.setLogFile("data/smartwaste.log");

    LOG_INFO("Main", "==================================================");
    LOG_INFO("Main", "Starting SmartWaste — Industrial Sorting Machine Demo");
    LOG_INFO("Main", "==================================================");

    if (logFileOk) {
        LOG_INFO("Main", "Logging to file enabled: data/smartwaste.log");
    } else {
        LOG_WARN("Main", "Could not open data/smartwaste.log for file logging. Console logging only.");
    }

    // 2. Load Configuration
    ConfigManager configMgr;
    try {
        LOG_INFO("Config", "Loading system configuration from config/config.json...");
        configMgr.loadFromFile("config/config.json");
        LOG_INFO("Config", "Configuration loaded and validated successfully!");

        const auto& mConfig = configMgr.getMachineConfig();
        const auto& cConfig = configMgr.getCameraConfig();
        const auto& sConfig = configMgr.getSortingConfig();

        LOG_INFO("Config", "Machine Name: " + mConfig.name);
        LOG_INFO("Config", "Max Conveyor Speed: " + std::to_string(mConfig.maxConveyorSpeed) + " m/s");
        LOG_INFO("Config", "Camera Frame Dimensions: " + std::to_string(cConfig.frameWidth) + "x" + std::to_string(cConfig.frameHeight) + " @ " + std::to_string(cConfig.fps) + " FPS");
        LOG_INFO("Config", "Default Plastic Gate: Gate #" + std::to_string(sConfig.plasticGate));

    } catch (const ConfigException& ex) {
        LOG_ERROR("Config", std::string("Configuration load failed: ") + ex.what());
        return 1;
    }

    // 2b. Database Initialization Demonstration (Phase 7 Step 1)
    LOG_INFO("Database", "--- Initializing SQLite Database Foundation ---");
    smartwaste::database::SQLiteDatabase db;
    std::string dbPath = configMgr.getDatabasePath();
    if (db.open(dbPath)) {
        LOG_INFO("Database", "DATABASE INITIALIZATION: SUCCESS");
        LOG_INFO("Database", "DATABASE PATH: " + db.getDatabasePath());
        LOG_INFO("Database", "SCHEMA VERSION: " + std::to_string(db.getSchemaVersion()));
        LOG_INFO("Database", "DATABASE READY: TRUE");
    } else {
        LOG_ERROR("Database", "DATABASE INITIALIZATION FAILED: " + db.getLastError());
    }


    // 3. Multi-Gate Hardware, Telemetry, Safety Controller & FSM Setup
    LOG_INFO("Hardware", "--- Initializing Hardware, Safety Controller & Machine FSM ---");
    
    SimulatedConveyor conveyor(configMgr.getMachineConfig().maxConveyorSpeed);
    SimulatedMotor motor(1750.0);

    SimulatedGate gatePlastic(configMgr.getSortingConfig().plasticGate); // Gate #1
    SimulatedGate gateMetal(configMgr.getSortingConfig().metalGate);     // Gate #2
    SimulatedGate gatePaper(configMgr.getSortingConfig().paperGate);     // Gate #3

    SimulatedSensor sensor1(0.65);
    SimulatedSensor sensor2(0.65);

    HardwareManager hwManager(conveyor, motor);
    hwManager.addGate(gatePlastic);
    hwManager.addGate(gateMetal);
    hwManager.addGate(gatePaper);

    hwManager.addSensor(sensor1);
    hwManager.addSensor(sensor2);

    SystemHealthMonitor healthMonitor;
    SafetyController safetyController(healthMonitor);
    MachineFSM fsm(hwManager, safetyController);
    HardwareTelemetryAggregator aggregator(conveyor, motor, sensor1, gatePlastic);

    // Initial FSM State Observation
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    // FSM Lifecycle Execution: IDLE -> STARTING -> RUNNING
    LOG_INFO("FSM", "Executing FSM start command...");
    fsm.start();
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    // Pause & Resume Cycle: RUNNING -> PAUSED -> RUNNING
    LOG_INFO("FSM", "Executing FSM pause command...");
    fsm.pause();
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    LOG_INFO("FSM", "Executing FSM resume command...");
    fsm.resume();
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    // Actuate Gate via HardwareManager
    hwManager.actuateGate(0);

    // Controlled Shutdown: RUNNING -> STOPPING -> IDLE
    LOG_INFO("FSM", "Executing FSM stop command...");
    fsm.stop();
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    // Emergency Stop Sequence: IDLE -> STARTING -> RUNNING -> EMERGENCY_STOP -> IDLE
    LOG_INFO("FSM", "Restarting machine for Emergency Stop Demonstration...");
    fsm.start();
    LOG_INFO("FSM", "Asserting Emergency Stop!");
    fsm.emergencyStop();
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    LOG_INFO("FSM", "Executing Reset after Emergency Stop...");
    fsm.reset();
    LOG_INFO("FSM", "Current Machine State: " + fsm.getStateString());

    // 4. Demonstrate Enum String Conversions
    LOG_INFO("Demo", "--- Demonstrating WasteType Conversions ---");
    WasteType types[] = {WasteType::PLASTIC, WasteType::METAL, WasteType::PAPER, WasteType::GLASS, WasteType::OTHER, WasteType::UNKNOWN};
    for (WasteType t : types) {
        std::string str = wasteTypeToString(t);
        WasteType parsedBack = stringToWasteType(str);
        LOG_DEBUG("Demo", "Enum: " + str + " -> Parsed Back: " + wasteTypeToString(parsedBack));
    }

    // Test invalid string mapping
    std::string invalidTypeStr = "RUBBISH_UNKNOWN_TYPE";
    WasteType parsedInvalid = stringToWasteType(invalidTypeStr);
    LOG_WARN("Demo", "Invalid string '" + invalidTypeStr + "' mapped safely to: " + wasteTypeToString(parsedInvalid));

    LOG_INFO("Demo", "--- Demonstrating MachineState Conversions ---");
    MachineState states[] = {MachineState::IDLE, MachineState::STARTING, MachineState::RUNNING, MachineState::PAUSED, MachineState::STOPPING, MachineState::ERROR, MachineState::EMERGENCY_STOP};
    for (MachineState s : states) {
        std::string str = machineStateToString(s);
        MachineState parsedBack = stringToMachineState(str);
        LOG_DEBUG("Demo", "State: " + str + " -> Parsed Back: " + machineStateToString(parsedBack));
    }

    // 5. Demonstrate Sample Domain Objects
    LOG_INFO("Demo", "--- Demonstrating Domain Models ---");
    
    // Create SystemStatus snapshot
    SystemStatus status;
    status.state = fsm.getCurrentState();
    status.isCameraConnected = true;
    status.isConveyorRunning = (conveyor.getSpeed() > 0.0);
    status.isMotorNormal = (motor.getRPM() > 0.0);
    status.isEmergencyStopActive = (fsm.getCurrentState() == MachineState::EMERGENCY_STOP);
    status.currentDetectedWaste = WasteType::PLASTIC;
    status.totalProcessedItems = 42;

    LOG_INFO("Domain", status.toString());

    // Create detected WasteItem
    WasteItem item;
    item.id = "ITEM-20260922-001";
    item.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    item.type = WasteType::PLASTIC;
    item.confidence = 0.94;
    item.boundingBox = BoundingBox{120, 340, 200, 180};
    item.gateId = configMgr.getSortingConfig().plasticGate;
    item.sortingStatus = "PENDING";

    LOG_INFO("Domain", item.toString());

    // Create SortingResult
    SortingResult result;
    result.type = item.type;
    result.targetGate = item.gateId;
    result.confidence = item.confidence;
    result.status = "SUCCESS";
    result.message = "Pneumatic diverter gate #1 actuated cleanly.";

    LOG_INFO("Domain", result.toString());

    // 6. Demonstrate Sorting Pipeline & Hardware Integration (Phase 4 Step 2B)
    LOG_INFO("Demo", "--- Demonstrating Sorting Pipeline ---");
    SortingController sortingController(configMgr);

    WasteItem samplePlastic;
    samplePlastic.id = "ITEM-PLASTIC-001";
    samplePlastic.type = WasteType::PLASTIC;
    samplePlastic.confidence = 0.94;

    WasteItem sampleMetal;
    sampleMetal.id = "ITEM-METAL-002";
    sampleMetal.type = WasteType::METAL;
    sampleMetal.confidence = 0.89;

    WasteItem samplePaper;
    samplePaper.id = "ITEM-PAPER-003";
    samplePaper.type = WasteType::PAPER;
    samplePaper.confidence = 0.82;

    WasteItem sampleLowConf;
    sampleLowConf.id = "ITEM-LOWCONF-004";
    sampleLowConf.type = WasteType::PLASTIC;
    sampleLowConf.confidence = 0.45;

    WasteItem sampleItems[] = {samplePlastic, sampleMetal, samplePaper, sampleLowConf};

    for (const auto& wItem : sampleItems) {
        LOG_INFO("Sorting", "Detected " + wasteTypeToString(wItem.type) + " with confidence " + std::to_string(static_cast<int>(wItem.confidence * 100)) + "%");
        SortingResult sResult = sortingController.processAndRoute(wItem, hwManager);
        LOG_INFO("Sorting", "Target Gate: #" + std::to_string(sResult.targetGate));
        if (sResult.status == "SUCCESS") {
            LOG_INFO("Sorting", "Gate #" + std::to_string(sResult.targetGate) + " actuated successfully.");
        } else {
            LOG_WARN("Sorting", "Gate actuation skipped or failed: " + sResult.status + " - " + sResult.message);
        }
    }

    // 7. Demonstrate Automated Sorting Simulation Pipeline (Phase 4 Step 3)
    LOG_INFO("Demo", "--- Automated Sorting Simulation ---");

    std::vector<WasteItem> simItems = {
        WasteItem{"ITEM-001", 0, WasteType::PLASTIC, 0.94, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"ITEM-002", 0, WasteType::METAL,   0.91, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"ITEM-003", 0, WasteType::PAPER,   0.88, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"ITEM-004", 0, WasteType::PLASTIC, 0.96, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"ITEM-005", 0, WasteType::PLASTIC, 0.40, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"ITEM-006", 0, WasteType::GLASS,   0.93, BoundingBox{0,0,0,0}, 0, "PENDING"}
    };

    SimulationRunner autoSimRunner;
    SimulationStatistics simStats = autoSimRunner.runSimulation(simItems, sortingController, hwManager);

    std::cout << "\n" << simStats.toString() << "\n\n";

    // 8. Demonstrate Telemetry & System Health Monitoring Integration (Phase 4 Step 4)
    LOG_INFO("Demo", "--- Telemetry & System Health Monitoring ---");

    HardwareTelemetryAggregator telemetryAggregator(conveyor, motor, sensor1, gatePlastic);

    SimulationRunner telemetrySimRunner;

    std::vector<WasteItem> telemetryItemsNormal = {
        WasteItem{"ITEM-001", 0, WasteType::PLASTIC, 0.94, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"ITEM-002", 0, WasteType::METAL,   0.91, BoundingBox{0,0,0,0}, 0, "PENDING"}
    };

    telemetrySimRunner.runSimulation(telemetryItemsNormal, sortingController, hwManager, &telemetryAggregator, &healthMonitor);

    LOG_INFO("Demo", "--- Simulated Hardware Fault ---");
    LOG_INFO("Hardware", "Triggering mechanical diverter gate jam fault...");
    gatePlastic.setJamDetected(true);

    WasteItem faultItem{"ITEM-FAULT-003", 0, WasteType::PLASTIC, 0.95, BoundingBox{0,0,0,0}, 0, "PENDING"};
    telemetrySimRunner.processItem(faultItem, sortingController, hwManager, &telemetryAggregator, &healthMonitor);

    gatePlastic.clearJam();

    std::cout << "\n" << telemetrySimRunner.getStatistics().toString() << "\n\n";

    // 9. Demonstrate Operational Event History & Event Tracking (Phase 4 Step 5)
    LOG_INFO("Demo", "--- Operational Event History ---");

    EventHistory eventHistory;
    fsm.setEventHistory(&eventHistory);

    // Demonstrate state lifecycle events
    fsm.start();
    fsm.stop();
    fsm.start();
    fsm.emergencyStop();
    fsm.reset();

    // Process a batch of items recording events into history
    std::vector<WasteItem> eventItems = {
        WasteItem{"EVT-ITEM-001", 0, WasteType::PLASTIC, 0.94, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"EVT-ITEM-002", 0, WasteType::METAL,   0.90, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"EVT-ITEM-003", 0, WasteType::PLASTIC, 0.40, BoundingBox{0,0,0,0}, 0, "PENDING"},
        WasteItem{"EVT-ITEM-004", 0, WasteType::GLASS,   0.92, BoundingBox{0,0,0,0}, 0, "PENDING"}
    };

    SimulationRunner eventSimRunner;
    eventSimRunner.runSimulation(eventItems, sortingController, hwManager, &telemetryAggregator, &healthMonitor, &eventHistory);

    // Demonstrate fault event recording
    gatePlastic.setJamDetected(true);
    WasteItem jamEvtItem{"EVT-ITEM-JAM", 0, WasteType::PLASTIC, 0.96, BoundingBox{0,0,0,0}, 0, "PENDING"};
    eventSimRunner.processItem(jamEvtItem, sortingController, hwManager, &telemetryAggregator, &healthMonitor, &eventHistory);
    gatePlastic.clearJam();

    // Display recent events
    std::cout << "\n--- Recent Operational Events (Latest 10) ---\n";
    auto recentEvts = eventHistory.getRecentEvents(10);
    for (const auto& evt : recentEvts) {
        std::cout << evt.toString() << "\n";
    }

    // Display event statistics summary
    std::cout << "\n--- Event Summary ---\n"
              << "Total Events: " << eventHistory.getEventCount() << "\n"
              << "Sorting Events: " << (eventHistory.getEventsByType(EventType::ITEM_SORTED).size() +
                                        eventHistory.getEventsByType(EventType::ITEM_REJECTED).size() +
                                        eventHistory.getEventsByType(EventType::GATE_UNAVAILABLE).size()) << "\n"
              << "Hardware Events: " << (eventHistory.getEventsByType(EventType::GATE_ACTUATED).size() +
                                         eventHistory.getEventsByType(EventType::GATE_JAMMED).size()) << "\n"
              << "Health Events: " << (eventHistory.getEventsByType(EventType::HEALTH_WARNING).size() +
                                       eventHistory.getEventsByType(EventType::HEALTH_FAULT).size()) << "\n"
              << "Safety Events: " << (eventHistory.getEventsByType(EventType::MACHINE_STARTED).size() +
                                       eventHistory.getEventsByType(EventType::MACHINE_STOPPED).size() +
                                       eventHistory.getEventsByType(EventType::EMERGENCY_STOP).size() +
                                       eventHistory.getEventsByType(EventType::MACHINE_RESET).size()) << "\n\n";

    // 10. Demonstrate Persistent Event Storage (Phase 4 Step 6)
    LOG_INFO("Demo", "--- Persistent Event Storage Demo ---");

    FileEventRepository fileRepo(configMgr.getEventStoragePath());
    EventHistory persistentHistory(&fileRepo);

    std::size_t loadedEventsCount = persistentHistory.loadFromRepository();
    LOG_INFO("Persistence", "Loaded " + std::to_string(loadedEventsCount) + " existing operational events from: " + fileRepo.getFilePath());

    // Add new operational events with auto-persistence
    persistentHistory.addEvent(EventType::MACHINE_STARTED, LogLevel::LOG_INFO_LEVEL, "SmartWaste machine initialized with persistent event store");
    persistentHistory.addEvent(EventType::ITEM_SORTED, LogLevel::LOG_INFO_LEVEL, "Item EVT-DEMO-001 sorted to Gate 1", "EVT-DEMO-001", WasteType::PLASTIC, 1);

    std::cout << "\n--- Persisted Events in Storage (" << fileRepo.getFilePath() << ") ---\n";
    auto loadedEvts = fileRepo.loadEvents();
    for (const auto& evt : loadedEvts) {
        std::cout << evt.toString() << "\n";
    }

    // Demonstrate Event ID Continuity across restart simulation
    LOG_INFO("Demo", "--- Demonstrating Event ID Continuity Across Application Restart ---");
    EventHistory restartedHistory(&fileRepo);
    restartedHistory.loadFromRepository();

    OperationalEvent postRestartEvt = restartedHistory.addEvent(EventType::HEALTH_WARNING, LogLevel::LOG_WARN_LEVEL, "Post-restart system health check warning");
    LOG_INFO("Persistence", "New event generated after restart: ID=" + postRestartEvt.eventId + " Message='" + postRestartEvt.message + "'");

    // 11. Demonstrate SmartWaste Engine Integration (Phase 4 Step 7)
    LOG_INFO("Demo", "--- SmartWaste Engine Integration Demo ---");

    SmartWasteEngine engine("data/demo_engine_events.jsonl");
    engine.initialize("config/config.json");

    LOG_INFO("EngineDemo", "Engine initialized. State: " + engine.getMachineStateString());

    // Start engine lifecycle
    engine.startMachine();
    LOG_INFO("EngineDemo", "Engine started. State: " + engine.getMachineStateString());

    // Process sample waste item through engine facade
    WasteItem demoItem{"ENG-DEMO-001", 0, WasteType::PLASTIC, 0.96, BoundingBox{0,0,0,0}, 0, "PENDING"};
    SortingResult demoRes = engine.processWasteItem(demoItem);
    LOG_INFO("EngineDemo", "Processed item ENG-DEMO-001 -> Gate #" + std::to_string(demoRes.targetGate) + " Status: " + demoRes.status);

    // Retrieve and display unified EngineSnapshot
    EngineSnapshot engineSnap = engine.getSnapshot();
    std::cout << "\n" << engineSnap.toString() << "\n\n";

    // Stop engine and clean up demo storage
    engine.stopMachine();
    LOG_INFO("EngineDemo", "Engine stopped cleanly. State: " + engine.getMachineStateString());
    std::remove("data/demo_engine_events.jsonl");

    // 12. Demonstrate Native C++ HTTP Server Foundation (Phase 5 Step 2)
    LOG_INFO("Demo", "--- Native C++ HTTP Server Foundation Demo ---");

    ServerConfig apiConfig{"127.0.0.1", 8080, 1048576, 10};
    HttpServer apiServer(apiConfig);

    bool apiStarted = apiServer.start();
    if (apiStarted) {
        LOG_INFO("ApiDemo", "SmartWasteApiServer started listening on http://127.0.0.1:8080");
        LOG_INFO("ApiDemo", "Registered route: GET /api/v1/ping");

        // Dispatch GET /api/v1/ping route request
        HttpRequest pingReq;
        pingReq.method = "GET";
        pingReq.path = "/api/v1/ping";
        HttpResponse pingRes = apiServer.getRouter().routeRequest(pingReq);
        LOG_INFO("ApiDemo", "GET /api/v1/ping response status: " + std::to_string(pingRes.statusCode) + " Body: " + pingRes.body);

        apiServer.stop();
        LOG_INFO("ApiDemo", "SmartWasteApiServer stopped cleanly.");
    } else {
        LOG_WARN("ApiDemo", "SmartWasteApiServer failed to bind port 8080 (port may be in use).");
    }

    // 13. Demonstrate Machine Lifecycle REST API (Phase 5 Step 3)
    LOG_INFO("Demo", "--- Machine Lifecycle REST API Demo ---");
    SmartWasteEngine apiEngine("data/demo_api_events.jsonl");
    apiEngine.initialize("config/config.json");

    ServerConfig machineApiConfig{"127.0.0.1", 8080, 1048576, 10};
    HttpServer machineApiServer(machineApiConfig);
    machineApiServer.getRouter().setEngine(&apiEngine);

    if (machineApiServer.start()) {
        LOG_INFO("ApiDemo", "SmartWasteApiServer started with MachineController endpoints registered.");

        ApiRouter& router = machineApiServer.getRouter();

        // GET /api/v1/machine/status
        HttpRequest reqStatus;
        reqStatus.method = "GET";
        reqStatus.path = "/api/v1/machine/status";
        HttpResponse resStatus = router.routeRequest(reqStatus);
        LOG_INFO("ApiDemo", "GET /api/v1/machine/status -> " + std::to_string(resStatus.statusCode) + " " + resStatus.body);

        // POST /api/v1/machine/start
        HttpRequest reqStart;
        reqStart.method = "POST";
        reqStart.path = "/api/v1/machine/start";
        HttpResponse resStart = router.routeRequest(reqStart);
        LOG_INFO("ApiDemo", "POST /api/v1/machine/start -> " + std::to_string(resStart.statusCode) + " " + resStart.body);

        // POST /api/v1/machine/pause
        HttpRequest reqPause;
        reqPause.method = "POST";
        reqPause.path = "/api/v1/machine/pause";
        HttpResponse resPause = router.routeRequest(reqPause);
        LOG_INFO("ApiDemo", "POST /api/v1/machine/pause -> " + std::to_string(resPause.statusCode) + " " + resPause.body);

        // POST /api/v1/machine/resume
        HttpRequest reqResume;
        reqResume.method = "POST";
        reqResume.path = "/api/v1/machine/resume";
        HttpResponse resResume = router.routeRequest(reqResume);
        LOG_INFO("ApiDemo", "POST /api/v1/machine/resume -> " + std::to_string(resResume.statusCode) + " " + resResume.body);

        // POST /api/v1/machine/stop
        HttpRequest reqStop;
        reqStop.method = "POST";
        reqStop.path = "/api/v1/machine/stop";
        HttpResponse resStop = router.routeRequest(reqStop);
        LOG_INFO("ApiDemo", "POST /api/v1/machine/stop -> " + std::to_string(resStop.statusCode) + " " + resStop.body);

        // POST /api/v1/machine/emergency-stop
        HttpRequest reqEStop;
        reqEStop.method = "POST";
        reqEStop.path = "/api/v1/machine/emergency-stop";
        HttpResponse resEStop = router.routeRequest(reqEStop);
        LOG_INFO("ApiDemo", "POST /api/v1/machine/emergency-stop -> " + std::to_string(resEStop.statusCode) + " " + resEStop.body);

        // POST /api/v1/machine/reset
        HttpRequest reqReset;
        reqReset.method = "POST";
        reqReset.path = "/api/v1/machine/reset";
        HttpResponse resReset = router.routeRequest(reqReset);
        LOG_INFO("ApiDemo", "POST /api/v1/machine/reset -> " + std::to_string(resReset.statusCode) + " " + resReset.body);

        // 14. Demonstrate Sorting, Telemetry, Health, Events & Simulation REST APIs (Phase 5 Step 4)
        LOG_INFO("Demo", "--- Sorting, Telemetry, Health, Events & Simulation REST API Demo ---");

        // POST /api/v1/sorting/process
        HttpRequest reqSort;
        reqSort.method = "POST";
        reqSort.path = "/api/v1/sorting/process";
        reqSort.body = R"({"id":"DEMO-PL-101","type":"PLASTIC","confidence":0.96,"gateId":1})";
        HttpResponse resSort = router.routeRequest(reqSort);
        LOG_INFO("ApiDemo", "POST /api/v1/sorting/process -> " + std::to_string(resSort.statusCode) + " " + resSort.body);

        // GET /api/v1/sorting/stats
        HttpRequest reqStats;
        reqStats.method = "GET";
        reqStats.path = "/api/v1/sorting/stats";
        HttpResponse resStats = router.routeRequest(reqStats);
        LOG_INFO("ApiDemo", "GET /api/v1/sorting/stats -> " + std::to_string(resStats.statusCode) + " " + resStats.body);

        // GET /api/v1/telemetry/latest
        HttpRequest reqTelem;
        reqTelem.method = "GET";
        reqTelem.path = "/api/v1/telemetry/latest";
        HttpResponse resTelem = router.routeRequest(reqTelem);
        LOG_INFO("ApiDemo", "GET /api/v1/telemetry/latest -> " + std::to_string(resTelem.statusCode) + " " + resTelem.body);

        // GET /api/v1/health
        HttpRequest reqHealth;
        reqHealth.method = "GET";
        reqHealth.path = "/api/v1/health";
        HttpResponse resHealth = router.routeRequest(reqHealth);
        LOG_INFO("ApiDemo", "GET /api/v1/health -> " + std::to_string(resHealth.statusCode) + " " + resHealth.body);

        // GET /api/v1/events
        HttpRequest reqEvts;
        reqEvts.method = "GET";
        reqEvts.path = "/api/v1/events";
        HttpResponse resEvts = router.routeRequest(reqEvts);
        LOG_INFO("ApiDemo", "GET /api/v1/events -> " + std::to_string(resEvts.statusCode) + " " + resEvts.body);

        // POST /api/v1/simulation/run
        HttpRequest reqSim;
        reqSim.method = "POST";
        reqSim.path = "/api/v1/simulation/run";
        reqSim.body = R"({"items":[{"id":"BATCH-1","type":"PLASTIC","confidence":0.92},{"id":"BATCH-2","type":"METAL","confidence":0.89}]})";
        HttpResponse resSim = router.routeRequest(reqSim);
        LOG_INFO("ApiDemo", "POST /api/v1/simulation/run -> " + std::to_string(resSim.statusCode) + " " + resSim.body);

        machineApiServer.stop();
        LOG_INFO("ApiDemo", "All SmartWaste REST API endpoints demonstrated and stopped cleanly.");
    }
    std::remove("data/demo_api_events.jsonl");

    // 15. Demonstrate Logging Levels
    LOG_INFO("Demo", "--- Demonstrating Logging Severity Levels ---");
    LOG_DEBUG("LoggerTest", "Debug message: Visualizing raw contour points.");
    LOG_INFO("LoggerTest", "Info message: Item routed to Gate #1.");
    LOG_WARN("LoggerTest", "Warning message: Camera dropped 2 frames under load.");
    LOG_ERROR("LoggerTest", "Error message: Proximity sensor #3 timeout.");
    LOG_CRITICAL("LoggerTest", "Critical message: Emergency Stop engaged by safety interlock!");

    LOG_INFO("Main", "SmartWaste Phase 5 Step 4 Demo finished successfully.");
    return 0;
}
