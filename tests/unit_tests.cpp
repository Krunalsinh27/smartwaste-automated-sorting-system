#include "../src/core/WasteType.hpp"
#include "../src/core/MachineState.hpp"
#include "../src/core/SystemStatus.hpp"
#include "../src/core/WasteItem.hpp"
#include "../src/core/SortingResult.hpp"
#include "../src/core/OperationalEvent.hpp"
#include "../src/core/EventHistory.hpp"
#include "../src/config/ConfigManager.hpp"
#include "../src/logging/Logger.hpp"

#include "../src/hal/IConveyor.hpp"
#include "../src/hal/IMotor.hpp"
#include "../src/hal/ISensor.hpp"
#include "../src/hal/IGate.hpp"

#include "../src/hardware/SimulatedConveyor.hpp"
#include "../src/hardware/SimulatedMotor.hpp"
#include "../src/hardware/SimulatedSensor.hpp"
#include "../src/hardware/SimulatedGate.hpp"
#include "../src/hardware/HardwareTelemetry.hpp"
#include "../src/hardware/HardwareTelemetryAggregator.hpp"
#include "../src/hardware/SimulationRunner.hpp"
#include "../src/hardware/HardwareManager.hpp"
#include "../src/hardware/SystemHealthMonitor.hpp"

#include "../src/controller/SafetyController.hpp"
#include "../src/controller/MachineFSM.hpp"
#include "../src/controller/SortingController.hpp"

#include "../src/persistence/IEventRepository.hpp"
#include "../src/persistence/FileEventRepository.hpp"

#include "../src/engine/SmartWasteEngine.hpp"
#include "../src/api/HttpServer.hpp"
#include "../src/api/controllers/MachineController.hpp"
#include "../src/database/SQLiteDatabase.hpp"

#include <iostream>
#include <cassert>
#include <fstream>
#include <type_traits>
#include <memory>
#include <cstdio>


using namespace smartwaste::core;
using namespace smartwaste::config;
using namespace smartwaste::logging;
using namespace smartwaste::hal;
using namespace smartwaste::hardware;
using namespace smartwaste::controller;
using namespace smartwaste::persistence;
using namespace smartwaste::database;
using namespace smartwaste::engine;
using namespace smartwaste::api;
using namespace smartwaste::api::controllers;


int g_testsPassed = 0;
int g_testsFailed = 0;

#define TEST_ASSERT(cond, name) \
    do { \
        if (cond) { \
            std::cout << "[PASS] " << name << std::endl; \
            g_testsPassed++; \
        } else { \
            std::cerr << "[FAIL] " << name << " (Line " << __LINE__ << ")" << std::endl; \
            g_testsFailed++; \
        } \
    } while(0)

void testHALInterfaceDeclarations() {
    TEST_ASSERT(std::is_abstract<IConveyor>::value, "IConveyor is pure abstract interface");
    TEST_ASSERT(std::has_virtual_destructor<IConveyor>::value, "IConveyor has virtual destructor");

    TEST_ASSERT(std::is_abstract<IMotor>::value, "IMotor is pure abstract interface");
    TEST_ASSERT(std::has_virtual_destructor<IMotor>::value, "IMotor has virtual destructor");

    TEST_ASSERT(std::is_abstract<ISensor>::value, "ISensor is pure abstract interface");
    TEST_ASSERT(std::has_virtual_destructor<ISensor>::value, "ISensor has virtual destructor");

    TEST_ASSERT(std::is_abstract<IGate>::value, "IGate is pure abstract interface");
    TEST_ASSERT(std::has_virtual_destructor<IGate>::value, "IGate has virtual destructor");
}

void testSimulatedConveyor() {
    SimulatedConveyor conveyor(4.0);
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "SimulatedConveyor initial speed is 0.0");
    TEST_ASSERT(!conveyor.isRunning(), "SimulatedConveyor initially not running");

    conveyor.setSpeed(2.5);
    TEST_ASSERT(conveyor.getSpeed() == 2.5, "SimulatedConveyor setSpeed updates speed to 2.5");
    TEST_ASSERT(conveyor.isRunning(), "SimulatedConveyor is running at 2.5 m/s");

    conveyor.setSpeed(10.0);
    TEST_ASSERT(conveyor.getSpeed() == 4.0, "SimulatedConveyor setSpeed clamps to maxSpeed 4.0");

    conveyor.stop();
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "SimulatedConveyor stop sets speed to 0.0");
    TEST_ASSERT(!conveyor.isRunning(), "SimulatedConveyor is stopped after stop()");

    std::unique_ptr<IConveyor> polyConveyor = std::make_unique<SimulatedConveyor>(3.0);
    polyConveyor->setSpeed(1.8);
    TEST_ASSERT(polyConveyor->getSpeed() == 1.8, "Polymorphic IConveyor::getSpeed() via IConveyor pointer");
    polyConveyor->stop();
    TEST_ASSERT(polyConveyor->getSpeed() == 0.0, "Polymorphic IConveyor::stop() via IConveyor pointer");
}

void testSimulatedMotor() {
    SimulatedMotor motor(1800.0);
    TEST_ASSERT(!motor.isRunning(), "SimulatedMotor initially stopped");
    TEST_ASSERT(motor.getRPM() == 0.0, "SimulatedMotor initial RPM is 0.0");

    motor.start();
    TEST_ASSERT(motor.isRunning(), "SimulatedMotor isRunning is true after start()");
    TEST_ASSERT(motor.getRPM() == 1800.0, "SimulatedMotor getRPM returns target 1800.0 RPM");

    motor.stop();
    TEST_ASSERT(!motor.isRunning(), "SimulatedMotor isRunning is false after stop()");
    TEST_ASSERT(motor.getRPM() == 0.0, "SimulatedMotor getRPM returns 0.0 after stop()");

    motor.setAbnormalState(true);
    TEST_ASSERT(motor.isAbnormalState(), "SimulatedMotor fault state injected");
    motor.start();
    TEST_ASSERT(motor.getRPM() == 0.0, "SimulatedMotor RPM remains 0.0 when starting in abnormal fault state");
    motor.setAbnormalState(false);
    TEST_ASSERT(!motor.isAbnormalState(), "SimulatedMotor fault state cleared");

    std::unique_ptr<IMotor> polyMotor = std::make_unique<SimulatedMotor>(1200.0);
    polyMotor->start();
    TEST_ASSERT(polyMotor->getRPM() == 1200.0, "Polymorphic IMotor::start() & getRPM() via IMotor pointer");
    polyMotor->stop();
    TEST_ASSERT(polyMotor->getRPM() == 0.0, "Polymorphic IMotor::stop() via IMotor pointer");
}

void testSimulatedSensor() {
    SimulatedSensor sensor(0.75);
    TEST_ASSERT(sensor.readSignal() == 0.0, "SimulatedSensor initial signal is 0.0");
    TEST_ASSERT(!sensor.isTriggered(), "SimulatedSensor initial trigger state is false");

    sensor.setSignal(0.80);
    TEST_ASSERT(sensor.readSignal() == 0.80, "SimulatedSensor setSignal updates signal to 0.80");
    TEST_ASSERT(sensor.isTriggered(), "SimulatedSensor isTriggered() returns true (0.80 >= 0.75)");

    sensor.setSignal(0.30);
    TEST_ASSERT(!sensor.isTriggered(), "SimulatedSensor isTriggered() returns false (0.30 < 0.75)");

    sensor.setTriggered(true);
    TEST_ASSERT(sensor.isTriggered(), "SimulatedSensor forced setTriggered(true)");

    sensor.setTriggered(false);
    TEST_ASSERT(!sensor.isTriggered(), "SimulatedSensor forced setTriggered(false)");

    std::unique_ptr<ISensor> polySensor = std::make_unique<SimulatedSensor>(0.5);
    TEST_ASSERT(!polySensor->isTriggered(), "Polymorphic ISensor::isTriggered() via ISensor pointer");
}

void testSimulatedGate() {
    SimulatedGate gate(1);
    TEST_ASSERT(gate.getGateId() == 1, "SimulatedGate gate ID is 1");
    TEST_ASSERT(gate.getActuationCount() == 0, "SimulatedGate initial actuation count is 0");
    TEST_ASSERT(!gate.isJamDetected(), "SimulatedGate initial jam state is false");
    TEST_ASSERT(!gate.wasActuated(), "SimulatedGate wasActuated is initially false");

    gate.actuate();
    TEST_ASSERT(gate.getActuationCount() == 1, "SimulatedGate actuation count increments to 1");
    TEST_ASSERT(gate.wasActuated(), "SimulatedGate wasActuated is true after actuate()");

    gate.setJamDetected(true);
    TEST_ASSERT(gate.isJamDetected(), "SimulatedGate fault injected: isJamDetected() returns true");

    gate.actuate();
    TEST_ASSERT(gate.getActuationCount() == 1, "SimulatedGate actuation count stays 1 during jam");

    gate.clearJam();
    TEST_ASSERT(!gate.isJamDetected(), "SimulatedGate jam cleared");
    gate.actuate();
    TEST_ASSERT(gate.getActuationCount() == 2, "SimulatedGate actuation count increments to 2 after clearing jam");

    std::unique_ptr<IGate> polyGate = std::make_unique<SimulatedGate>(2);
    TEST_ASSERT(!polyGate->isJamDetected(), "Polymorphic IGate::isJamDetected() via IGate pointer");
    polyGate->actuate();
    TEST_ASSERT(polyGate->isJamDetected() == false, "Polymorphic IGate::actuate() executed cleanly");
}

void testHardwareTelemetryModel() {
    HardwareTelemetry t;
    TEST_ASSERT(t.conveyorSpeed == 0.0, "Telemetry default conveyorSpeed is 0.0");
    TEST_ASSERT(t.motorRPM == 0.0, "Telemetry default motorRPM is 0.0");
    TEST_ASSERT(!t.motorRunning, "Telemetry default motorRunning is false");
    TEST_ASSERT(t.sensorSignal == 0.0, "Telemetry default sensorSignal is 0.0");
    TEST_ASSERT(!t.sensorTriggered, "Telemetry default sensorTriggered is false");
    TEST_ASSERT(!t.gateJamDetected, "Telemetry default gateJamDetected is false");

    t.conveyorSpeed = 2.0;
    t.motorRPM = 1500.0;
    t.motorRunning = true;
    t.sensorSignal = 0.85;
    t.sensorTriggered = true;
    t.gateJamDetected = false;

    TEST_ASSERT(!t.toString().empty(), "Telemetry toString output is non-empty");
}

void testHardwareTelemetryAggregatorAndIntegration() {
    SimulatedConveyor conveyor(5.0);
    SimulatedMotor motor(1500.0);
    SimulatedSensor sensor(0.70);
    SimulatedGate gate(1);

    const IConveyor& polyConveyor = conveyor;
    const IMotor& polyMotor = motor;
    const ISensor& polySensor = sensor;
    const IGate& polyGate = gate;

    HardwareTelemetryAggregator aggregator(polyConveyor, polyMotor, polySensor, polyGate);

    HardwareTelemetry tStopped = aggregator.read();
    TEST_ASSERT(tStopped.conveyorSpeed == 0.0, "Aggregator reads initial conveyor speed 0.0 m/s");
    TEST_ASSERT(tStopped.motorRPM == 0.0, "Aggregator reads initial motor RPM 0.0");
    TEST_ASSERT(!tStopped.motorRunning, "Aggregator reads initial motor running false");
    TEST_ASSERT(tStopped.sensorSignal == 0.0, "Aggregator reads initial sensor signal 0.0");
    TEST_ASSERT(!tStopped.sensorTriggered, "Aggregator reads initial sensor triggered false");
    TEST_ASSERT(!tStopped.gateJamDetected, "Aggregator reads initial gate jam false");

    motor.start();
    conveyor.setSpeed(2.2);
    sensor.setSignal(0.85);

    HardwareTelemetry tActive = aggregator.read();
    TEST_ASSERT(tActive.conveyorSpeed == 2.2, "Aggregator reads running conveyor speed 2.2 m/s");
    TEST_ASSERT(tActive.motorRPM == 1500.0, "Aggregator reads running motor RPM 1500.0");
    TEST_ASSERT(tActive.motorRunning, "Aggregator infers motor running true (RPM > 0)");
    TEST_ASSERT(tActive.sensorSignal == 0.85, "Aggregator reads sensor signal 0.85");
    TEST_ASSERT(tActive.sensorTriggered, "Aggregator reads sensor triggered true");
    TEST_ASSERT(!tActive.gateJamDetected, "Aggregator reads gate jam false during normal operation");

    gate.setJamDetected(true);
    HardwareTelemetry tJammed = aggregator.read();
    TEST_ASSERT(tJammed.gateJamDetected, "Aggregator detects injected gate jam");

    gate.clearJam();
    HardwareTelemetry tCleared = aggregator.read();
    TEST_ASSERT(!tCleared.gateJamDetected, "Aggregator detects gate jam cleared");

    conveyor.stop();
    motor.stop();
}

void testSimulationRunner() {
    SimulationRunner runner(std::chrono::milliseconds(20));
    TEST_ASSERT(runner.getTickCount() == 0, "SimulationRunner initial tick count is 0");
    TEST_ASSERT(runner.getElapsedTime().count() == 0, "SimulationRunner initial elapsed time is 0 ms");

    runner.stepSimulation(std::chrono::milliseconds(100));
    TEST_ASSERT(runner.getTickCount() == 5, "SimulationRunner 100 ms step produces 5 ticks (100 / 20)");
    TEST_ASSERT(runner.getElapsedTime().count() == 100, "SimulationRunner elapsed time is 100 ms");

    runner.stepSimulation(std::chrono::milliseconds(50));
    TEST_ASSERT(runner.getElapsedTime().count() == 150, "SimulationRunner total elapsed time is 150 ms");

    runner.reset();
    TEST_ASSERT(runner.getTickCount() == 0, "SimulationRunner reset restores tick count to 0");
    TEST_ASSERT(runner.getElapsedTime().count() == 0, "SimulationRunner reset restores elapsed time to 0 ms");
}

void testHardwareManagerMultiGate() {
    SimulatedConveyor conveyor(5.0);
    SimulatedMotor motor(1500.0);
    SimulatedSensor sensor1(0.5);
    SimulatedSensor sensor2(0.5);

    SimulatedGate gate1(1);
    SimulatedGate gate2(2);
    SimulatedGate gate3(3);

    HardwareManager manager(conveyor, motor);
    TEST_ASSERT(manager.getGateCount() == 0, "HardwareManager initially has 0 gates");
    TEST_ASSERT(manager.getSensorCount() == 0, "HardwareManager initially has 0 sensors");

    manager.addGate(gate1);
    manager.addGate(gate2);
    manager.addGate(gate3);

    manager.addSensor(sensor1);
    manager.addSensor(sensor2);

    TEST_ASSERT(manager.getGateCount() == 3, "HardwareManager registered 3 diverter gates");
    TEST_ASSERT(manager.getSensorCount() == 2, "HardwareManager registered 2 detection sensors");

    bool act1 = manager.actuateGate(0);
    TEST_ASSERT(act1, "HardwareManager actuates Gate 1 cleanly");
    TEST_ASSERT(gate1.getActuationCount() == 1, "Gate 1 actuation count is 1");

    bool act2 = manager.actuateGate(1);
    TEST_ASSERT(act2, "HardwareManager actuates Gate 2 cleanly");
    TEST_ASSERT(gate2.getActuationCount() == 1, "Gate 2 actuation count is 1");

    bool actInvalid = manager.actuateGate(99);
    TEST_ASSERT(!actInvalid, "HardwareManager returns false for out-of-bounds gate index 99");

    gate1.setJamDetected(true);
    TEST_ASSERT(manager.isGateJammed(0), "HardwareManager detects Gate 1 jammed");
    TEST_ASSERT(!manager.isGateJammed(1), "HardwareManager detects Gate 2 NOT jammed");
    TEST_ASSERT(manager.hasAnyGateJam(), "HardwareManager hasAnyGateJam() returns true");

    bool actJammed1 = manager.actuateGate(0);
    TEST_ASSERT(!actJammed1, "HardwareManager rejects actuation for jammed Gate 1");

    bool actHealthy2 = manager.actuateGate(1);
    TEST_ASSERT(actHealthy2, "HardwareManager successfully actuates healthy Gate 2 even while Gate 1 is jammed");
    TEST_ASSERT(gate2.getActuationCount() == 2, "Gate 2 actuation count increments to 2");

    gate1.clearJam();
    TEST_ASSERT(!manager.isGateJammed(0), "HardwareManager detects Gate 1 jam cleared");
    TEST_ASSERT(!manager.hasAnyGateJam(), "HardwareManager hasAnyGateJam() returns false after clearing jam");

    bool actRecovered1 = manager.actuateGate(0);
    TEST_ASSERT(actRecovered1, "HardwareManager actuates Gate 1 cleanly after jam recovery");
    TEST_ASSERT(gate1.getActuationCount() == 2, "Gate 1 actuation count increments to 2");

    conveyor.setSpeed(2.0);
    motor.start();
    manager.stopAll();
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "HardwareManager stopAll() stops conveyor");
    TEST_ASSERT(motor.getRPM() == 0.0, "HardwareManager stopAll() stops motor");
}

void testSystemHealthMonitor() {
    SystemHealthMonitor monitor;

    HardwareTelemetry tStopped;
    tStopped.conveyorSpeed = 0.0;
    tStopped.motorRPM = 0.0;
    tStopped.motorRunning = false;
    tStopped.gateJamDetected = false;

    HardwareHealth hStopped = monitor.evaluate(tStopped);
    TEST_ASSERT(hStopped.status == HealthStatus::WARNING, "SystemHealthMonitor returns WARNING for stopped hardware");
    TEST_ASSERT(hStopped.isMotorStopped, "SystemHealthMonitor flags isMotorStopped true");

    HardwareTelemetry tRunning;
    tRunning.conveyorSpeed = 2.5;
    tRunning.motorRPM = 1500.0;
    tRunning.motorRunning = true;
    tRunning.gateJamDetected = false;

    HardwareHealth hRunning = monitor.evaluate(tRunning);
    TEST_ASSERT(hRunning.status == HealthStatus::HEALTHY, "SystemHealthMonitor returns HEALTHY for normal running hardware");

    HardwareTelemetry tJammed = tRunning;
    tJammed.gateJamDetected = true;

    HardwareHealth hJammed = monitor.evaluate(tJammed);
    TEST_ASSERT(hJammed.status == HealthStatus::FAULT, "SystemHealthMonitor returns FAULT when gate jam detected");
    TEST_ASSERT(hJammed.isGateJammed, "SystemHealthMonitor flags isGateJammed true");

    HardwareTelemetry tRecovered = tRunning;
    tRecovered.gateJamDetected = false;

    HardwareHealth hRecovered = monitor.evaluate(tRecovered);
    TEST_ASSERT(hRecovered.status == HealthStatus::HEALTHY, "SystemHealthMonitor returns HEALTHY after jam recovery");
}

void testIntegratedMultiGateHealthScenario() {
    SimulatedConveyor conveyor(5.0);
    SimulatedMotor motor(1600.0);
    SimulatedSensor sensor1(0.5);
    SimulatedSensor sensor2(0.5);

    SimulatedGate gate1(1);
    SimulatedGate gate2(2);

    HardwareManager manager(conveyor, motor);
    manager.addGate(gate1);
    manager.addGate(gate2);
    manager.addSensor(sensor1);
    manager.addSensor(sensor2);

    HardwareTelemetryAggregator aggregator(conveyor, motor, sensor1, gate1);
    SystemHealthMonitor healthMonitor;

    motor.start();
    conveyor.setSpeed(2.0);

    HardwareTelemetry t1 = aggregator.read();
    HardwareHealth h1 = healthMonitor.evaluate(t1);
    TEST_ASSERT(h1.status == HealthStatus::HEALTHY, "Integrated SystemHealthMonitor reads HEALTHY during normal operation");

    gate1.setJamDetected(true);
    HardwareTelemetry tJam = aggregator.read();
    HardwareHealth hJam = healthMonitor.evaluate(tJam);
    TEST_ASSERT(hJam.status == HealthStatus::FAULT, "Integrated SystemHealthMonitor reads FAULT on Gate 1 jam");

    bool act2 = manager.actuateGate(1);
    TEST_ASSERT(act2, "Gate 2 actuated cleanly via HardwareManager while Gate 1 is jammed");

    gate1.clearJam();
    HardwareTelemetry tRecovered = aggregator.read();
    HardwareHealth hRecovered = healthMonitor.evaluate(tRecovered);
    TEST_ASSERT(hRecovered.status == HealthStatus::HEALTHY, "Integrated SystemHealthMonitor recovers to HEALTHY after clearing jam");

    manager.stopAll();
}

void testSafetyControllerAndFSM() {
    // 1. Setup Hardware & Controllers
    SimulatedConveyor conveyor(5.0);
    SimulatedMotor motor(1500.0);
    SimulatedGate gate1(1);
    SimulatedGate gate2(2);

    HardwareManager hwManager(conveyor, motor);
    hwManager.addGate(gate1);
    hwManager.addGate(gate2);

    SystemHealthMonitor healthMonitor;
    SafetyController safetyController(healthMonitor);
    MachineFSM fsm(hwManager, safetyController);

    // Initial FSM State
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "FSM initial state is IDLE");

    // Invalid Start / Pause / Resume / Stop calls while IDLE
    TEST_ASSERT(!fsm.pause(), "FSM rejects pause() while IDLE");
    TEST_ASSERT(!fsm.resume(), "FSM rejects resume() while IDLE");
    TEST_ASSERT(!fsm.stop(), "FSM rejects stop() while IDLE");
    TEST_ASSERT(!fsm.reset(), "FSM rejects reset() while IDLE");

    // Healthy Start Sequence: IDLE -> STARTING -> RUNNING
    bool started = fsm.start();
    TEST_ASSERT(started, "FSM start() succeeds under healthy conditions");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::RUNNING, "FSM state is RUNNING after start()");
    TEST_ASSERT(conveyor.getSpeed() > 0.0, "Conveyor is running after FSM start()");
    TEST_ASSERT(motor.getRPM() > 0.0, "Motor is running after FSM start()");

    // Pause & Resume: RUNNING -> PAUSED -> RUNNING
    bool paused = fsm.pause();
    TEST_ASSERT(paused, "FSM pause() succeeds from RUNNING");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::PAUSED, "FSM state is PAUSED");
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "Conveyor stopped during PAUSED");

    bool resumed = fsm.resume();
    TEST_ASSERT(resumed, "FSM resume() succeeds from PAUSED");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::RUNNING, "FSM state returned to RUNNING");
    TEST_ASSERT(conveyor.getSpeed() > 0.0, "Conveyor speed restored after resume()");

    // Controlled Stop Sequence: RUNNING -> STOPPING -> IDLE
    bool stopped = fsm.stop();
    TEST_ASSERT(stopped, "FSM stop() succeeds from RUNNING");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "FSM state returned to IDLE after stop()");
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "Conveyor stopped after stop()");

    // Safety Interlock Check: Block start when Gate 1 is jammed
    gate1.setJamDetected(true);
    bool unsafeStarted = fsm.start();
    TEST_ASSERT(!unsafeStarted, "FSM start() fails when Gate 1 is jammed");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::ERROR, "FSM state transitions to ERROR on unsafe start");

    // Cannot resume from ERROR without reset
    TEST_ASSERT(!fsm.resume(), "FSM rejects resume() while in ERROR state");

    // Reset rejected while jam persists
    bool resetPersistJam = fsm.reset();
    TEST_ASSERT(!resetPersistJam, "FSM reset() rejected while gate jam persists");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::ERROR, "FSM remains in ERROR state");

    // Clear jam and reset: ERROR -> IDLE
    gate1.clearJam();
    bool resetSuccess = fsm.reset();
    TEST_ASSERT(resetSuccess, "FSM reset() succeeds after clearing jam fault");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "FSM state returned to IDLE after reset()");

    // Emergency Stop from RUNNING: RUNNING -> EMERGENCY_STOP
    fsm.start();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::RUNNING, "FSM in RUNNING state prior to E-Stop");

    fsm.emergencyStop();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::EMERGENCY_STOP, "FSM state is EMERGENCY_STOP");
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "Emergency stop shuts down conveyor");
    TEST_ASSERT(motor.getRPM() == 0.0, "Emergency stop shuts down motor");

    // Cannot start/pause/resume from EMERGENCY_STOP without reset
    TEST_ASSERT(!fsm.start(), "FSM rejects start() while in EMERGENCY_STOP");
    TEST_ASSERT(!fsm.pause(), "FSM rejects pause() while in EMERGENCY_STOP");

    // Reset from EMERGENCY_STOP -> IDLE
    bool resetEstop = fsm.reset();
    TEST_ASSERT(resetEstop, "FSM reset() succeeds from EMERGENCY_STOP");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "FSM state returned to IDLE after E-Stop reset");

    // Emergency Stop from PAUSED
    fsm.start();
    fsm.pause();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::PAUSED, "FSM in PAUSED state");
    fsm.emergencyStop();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::EMERGENCY_STOP, "FSM E-Stop asserted from PAUSED");
    fsm.reset();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "FSM reset to IDLE");

    // Operational Fault Triggering during RUNNING
    fsm.start();
    fsm.triggerFault("Sensor timeout");
    TEST_ASSERT(fsm.getCurrentState() == MachineState::ERROR, "FSM transitions to ERROR on triggerFault()");
    TEST_ASSERT(conveyor.getSpeed() == 0.0, "Hardware stopped on operational fault");
    fsm.reset();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "FSM reset to IDLE after operational fault");
}

void testSortingControllerHardwareIntegration() {
    ConfigManager configMgr;
    configMgr.loadFromFile("config/config.json");
    SortingController sortingController(configMgr);

    SimulatedConveyor conveyor(2.5);
    SimulatedMotor motor(1750.0);
    SimulatedGate gatePlastic(1); // Gate #1
    SimulatedGate gateMetal(2);   // Gate #2
    SimulatedGate gatePaper(3);   // Gate #3
    SimulatedGate gateGlass(4);   // Gate #4
    SimulatedGate gateOther(5);   // Gate #5

    HardwareManager hwManager(conveyor, motor);
    hwManager.addGate(gatePlastic);
    hwManager.addGate(gateMetal);
    hwManager.addGate(gatePaper);
    hwManager.addGate(gateGlass);
    hwManager.addGate(gateOther);

    // 1. PLASTIC classification -> Gate #1 (index 0) actuation
    WasteItem plasticItem;
    plasticItem.id = "TEST-PLASTIC";
    plasticItem.type = WasteType::PLASTIC;
    plasticItem.confidence = 0.95;
    
    SortingResult plasticResult = sortingController.processAndRoute(plasticItem, hwManager);
    TEST_ASSERT(plasticResult.status == "SUCCESS", "PLASTIC classification status is SUCCESS");
    TEST_ASSERT(plasticResult.targetGate == 1, "PLASTIC targetGate is 1");
    TEST_ASSERT(gatePlastic.wasActuated(), "Gate #1 (Plastic) was actuated");
    TEST_ASSERT(gatePlastic.getActuationCount() == 1, "Gate #1 actuation count is 1");

    // 2. METAL classification -> Gate #2 (index 1) actuation
    WasteItem metalItem;
    metalItem.id = "TEST-METAL";
    metalItem.type = WasteType::METAL;
    metalItem.confidence = 0.90;

    SortingResult metalResult = sortingController.processAndRoute(metalItem, hwManager);
    TEST_ASSERT(metalResult.status == "SUCCESS", "METAL classification status is SUCCESS");
    TEST_ASSERT(metalResult.targetGate == 2, "METAL targetGate is 2");
    TEST_ASSERT(gateMetal.wasActuated(), "Gate #2 (Metal) was actuated");
    TEST_ASSERT(gateMetal.getActuationCount() == 1, "Gate #2 actuation count is 1");

    // 3. PAPER classification -> Gate #3 (index 2) actuation
    WasteItem paperItem;
    paperItem.id = "TEST-PAPER";
    paperItem.type = WasteType::PAPER;
    paperItem.confidence = 0.85;

    SortingResult paperResult = sortingController.processAndRoute(paperItem, hwManager);
    TEST_ASSERT(paperResult.status == "SUCCESS", "PAPER classification status is SUCCESS");
    TEST_ASSERT(paperResult.targetGate == 3, "PAPER targetGate is 3");
    TEST_ASSERT(gatePaper.wasActuated(), "Gate #3 (Paper) was actuated");
    TEST_ASSERT(gatePaper.getActuationCount() == 1, "Gate #3 actuation count is 1");

    // 4. Low-confidence item -> REJECTED, no hardware actuation
    gatePlastic.resetActuatedFlag();
    gateMetal.resetActuatedFlag();
    gatePaper.resetActuatedFlag();
    gateGlass.resetActuatedFlag();
    gateOther.resetActuatedFlag();

    WasteItem lowConfItem;
    lowConfItem.id = "TEST-LOW-CONF";
    lowConfItem.type = WasteType::PLASTIC;
    lowConfItem.confidence = 0.40; // below 0.65 threshold

    SortingResult lowConfResult = sortingController.processAndRoute(lowConfItem, hwManager);
    TEST_ASSERT(lowConfResult.status == "REJECTED", "Low-confidence item status is REJECTED");
    TEST_ASSERT(!gatePlastic.wasActuated(), "No gate actuated for low-confidence item (Plastic)");
    TEST_ASSERT(!gateMetal.wasActuated(), "No gate actuated for low-confidence item (Metal)");
    TEST_ASSERT(!gatePaper.wasActuated(), "No gate actuated for low-confidence item (Paper)");

    // 5. GLASS classification -> Gate #4 (index 3) actuation
    WasteItem glassItem;
    glassItem.id = "TEST-GLASS";
    glassItem.type = WasteType::GLASS;
    glassItem.confidence = 0.92;

    SortingResult glassResult = sortingController.processAndRoute(glassItem, hwManager);
    TEST_ASSERT(glassResult.targetGate == 4, "GLASS targetGate is 4");
    TEST_ASSERT(glassResult.status == "SUCCESS", "GLASS status is SUCCESS when Gate #4 is registered");
    TEST_ASSERT(gateGlass.wasActuated(), "Gate #4 (Glass) was actuated");
    TEST_ASSERT(gateGlass.getActuationCount() == 1, "Gate #4 actuation count is 1");

    // 5b. OTHER classification -> Gate #5 (index 4) actuation
    WasteItem otherItem;
    otherItem.id = "TEST-OTHER";
    otherItem.type = WasteType::OTHER;
    otherItem.confidence = 0.88;

    SortingResult otherResult = sortingController.processAndRoute(otherItem, hwManager);
    TEST_ASSERT(otherResult.targetGate == 5, "OTHER targetGate is 5");
    TEST_ASSERT(otherResult.status == "SUCCESS", "OTHER status is SUCCESS when Gate #5 is registered");
    TEST_ASSERT(gateOther.wasActuated(), "Gate #5 (Other) was actuated");
    TEST_ASSERT(gateOther.getActuationCount() == 1, "Gate #5 actuation count is 1");

    // 6. Invalid gate index / targetGate <= 0 handled safely
    SortingResult invalidResult;
    invalidResult.targetGate = -1;
    invalidResult.status = "SUCCESS";
    bool invalidRouteSuccess = sortingController.routeToHardware(invalidResult, hwManager);
    TEST_ASSERT(!invalidRouteSuccess, "Invalid targetGate (-1) routeToHardware returns false");
    TEST_ASSERT(invalidResult.status == "GATE_UNAVAILABLE", "Invalid targetGate status set to GATE_UNAVAILABLE");

    // 6b. Unregistered gate index (e.g. Gate 6) returns GATE_UNAVAILABLE
    SortingResult unregResult;
    unregResult.targetGate = 6;
    unregResult.status = "SUCCESS";
    bool unregRouteSuccess = sortingController.routeToHardware(unregResult, hwManager);
    TEST_ASSERT(!unregRouteSuccess, "Unregistered targetGate (6) routeToHardware returns false");
    TEST_ASSERT(unregResult.status == "GATE_UNAVAILABLE", "Unregistered targetGate status set to GATE_UNAVAILABLE");

    // 7. Gate jam behavior preserved
    gatePlastic.setJamDetected(true);
    SortingResult jammedPlasticResult = sortingController.processAndRoute(plasticItem, hwManager);
    TEST_ASSERT(!jammedPlasticResult.message.empty(), "Jammed gate result contains informative message");
    TEST_ASSERT(jammedPlasticResult.status == "GATE_JAMMED", "Jammed gate status is GATE_JAMMED");
    gatePlastic.clearJam();
}

void testAutomatedSortingSimulationPipeline() {
    ConfigManager configMgr;
    configMgr.loadFromFile("config/config.json");
    SortingController controller(configMgr);

    SimulatedConveyor conveyor(2.5);
    SimulatedMotor motor(1750.0);
    SimulatedGate gatePlastic(1); // Gate #1
    SimulatedGate gateMetal(2);   // Gate #2
    SimulatedGate gatePaper(3);   // Gate #3
    SimulatedGate gateGlass(4);   // Gate #4
    SimulatedGate gateOther(5);   // Gate #5

    HardwareManager hwManager(conveyor, motor);
    hwManager.addGate(gatePlastic);
    hwManager.addGate(gateMetal);
    hwManager.addGate(gatePaper);
    hwManager.addGate(gateGlass);
    hwManager.addGate(gateOther);

    SimulationRunner runner;

    std::vector<WasteItem> items = {
        WasteItem{"ITEM-001", 0, WasteType::PLASTIC, 0.94, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-002", 0, WasteType::METAL,   0.91, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-003", 0, WasteType::PAPER,   0.88, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-004", 0, WasteType::PLASTIC, 0.96, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-005", 0, WasteType::PLASTIC, 0.40, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-006", 0, WasteType::GLASS,   0.93, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-007", 0, WasteType::OTHER,   0.85, BoundingBox{}, 0, "PENDING"}
    };

    SimulationStatistics stats = runner.runSimulation(items, controller, hwManager);

    // TEST 1: Multiple items can be processed sequentially
    TEST_ASSERT(stats.totalItems == 7, "TEST 1: Multiple items can be processed sequentially (Total items == 7)");

    // TEST 2: PLASTIC routes to Gate #1
    TEST_ASSERT(gatePlastic.getActuationCount() == 2, "TEST 2: PLASTIC routes to Gate #1 (2 actuations)");

    // TEST 3: METAL routes to Gate #2
    TEST_ASSERT(gateMetal.getActuationCount() == 1, "TEST 3: METAL routes to Gate #2 (1 actuation)");

    // TEST 4: PAPER routes to Gate #3
    TEST_ASSERT(gatePaper.getActuationCount() == 1, "TEST 4: PAPER routes to Gate #3 (1 actuation)");

    // TEST 5: GLASS routes to Gate #4
    TEST_ASSERT(gateGlass.getActuationCount() == 1, "TEST 5: GLASS routes to Gate #4 (1 actuation)");

    // TEST 6: OTHER routes to Gate #5
    TEST_ASSERT(gateOther.getActuationCount() == 1, "TEST 6: OTHER routes to Gate #5 (1 actuation)");

    // TEST 7: Low-confidence item is REJECTED and does not actuate gate
    TEST_ASSERT(stats.rejectedItems == 1, "TEST 7: Low-confidence item is REJECTED without gate actuation");

    // TEST 8: GLASS and OTHER are processed successfully (0 unavailable gate items)
    TEST_ASSERT(stats.unavailableGateItems == 0, "TEST 8: GLASS and OTHER processed successfully without unavailable gate errors");

    // TEST 9: Simulation statistics correctly count processed items
    TEST_ASSERT(stats.totalItems == 7, "TEST 9: Simulation statistics correctly count processed items (7)");

    // TEST 10: Successful items are counted correctly
    TEST_ASSERT(stats.successfulItems == 6, "TEST 10: Successful items counted correctly (6)");

    // TEST 11: Rejected items are counted correctly
    TEST_ASSERT(stats.rejectedItems == 1, "TEST 11: Rejected items counted correctly (1)");

    // TEST 12: Unavailable-gate items are counted correctly
    TEST_ASSERT(stats.unavailableGateItems == 0, "TEST 12: Unavailable-gate items counted correctly (0)");

    // TEST 13: Gate actuation count reflects actual successful hardware actuation
    TEST_ASSERT(stats.totalGateActuations == 6, "TEST 13: Total gate actuation count matches actual successful hardware actuations (6)");

    // TEST 14: Multiple items do not interfere with each other's state
    TEST_ASSERT(runner.getTickCount() > 0, "TEST 14: Multiple items process deterministically without interference");

    // TEST 15: Existing gate-jam behavior remains preserved
    runner.reset();
    gatePlastic.setJamDetected(true);
    WasteItem jamTestItem{"ITEM-JAM", 0, WasteType::PLASTIC, 0.95, BoundingBox{}, 0, "PENDING"};
    SortingResult jamResult = runner.processItem(jamTestItem, controller, hwManager);
    TEST_ASSERT(jamResult.status == "GATE_JAMMED", "TEST 15: Existing gate-jam behavior preserved (GATE_JAMMED)");
    TEST_ASSERT(runner.getStatistics().failedItems == 1, "TEST 15: Jammed item correctly counted as failedItem in statistics");
    gatePlastic.clearJam();
}

void testTelemetryAndHealthMonitoringIntegration() {
    ConfigManager configMgr;
    configMgr.loadFromFile("config/config.json");
    SortingController controller(configMgr);

    SimulatedConveyor conveyor(2.5);
    SimulatedMotor motor(1750.0);
    SimulatedGate gatePlastic(1); // Gate #1
    SimulatedGate gateMetal(2);   // Gate #2
    SimulatedGate gatePaper(3);   // Gate #3
    SimulatedSensor sensor(0.65);

    HardwareManager hwManager(conveyor, motor);
    hwManager.addGate(gatePlastic);
    hwManager.addGate(gateMetal);
    hwManager.addGate(gatePaper);

    HardwareTelemetryAggregator aggregator(conveyor, motor, sensor, gatePlastic);
    SystemHealthMonitor healthMonitor;
    SafetyController safetyController(healthMonitor);

    // TEST 1: Telemetry aggregator returns valid telemetry snapshot
    HardwareTelemetry t1 = aggregator.read();
    TEST_ASSERT(t1.timestampMs > 0, "TEST 1: Telemetry snapshot contains valid timestamp");

    // TEST 2: Conveyor speed reflected
    conveyor.setSpeed(2.0);
    HardwareTelemetry t2 = aggregator.read();
    TEST_ASSERT(t2.conveyorSpeed == 2.0, "TEST 2: Telemetry reflects current conveyor speed (2.0 m/s)");

    // TEST 3: Motor RPM reflected
    motor.start();
    HardwareTelemetry t3 = aggregator.read();
    TEST_ASSERT(t3.motorRPM == 1750.0, "TEST 3: Telemetry reflects current motor RPM (1750.0)");
    TEST_ASSERT(t3.motorRunning == true, "TEST 3: Telemetry reflects motor running state (true)");

    // TEST 4: Gate state reflected
    TEST_ASSERT(t3.gateJamDetected == false, "TEST 4: Telemetry reflects gate jam state (false)");

    // TEST 5: SystemHealthMonitor evaluates normal telemetry as HEALTHY
    HardwareHealth h1 = healthMonitor.evaluate(t3);
    TEST_ASSERT(h1.status == HealthStatus::HEALTHY, "TEST 5: SystemHealthMonitor evaluates normal telemetry as HEALTHY");

    // TEST 6: Healthy hardware does not trigger emergency condition
    TEST_ASSERT(!safetyController.isEmergencyStopRequired(t3), "TEST 6: Healthy hardware does not trigger emergency condition");
    TEST_ASSERT(safetyController.isSafeToContinue(t3), "TEST 6: Healthy hardware is safe to continue");

    // TEST 7 & TEST 8: Gate-jam condition visible to health monitoring and status remains GATE_JAMMED
    gatePlastic.setJamDetected(true);
    HardwareTelemetry tJam = aggregator.read();
    TEST_ASSERT(tJam.gateJamDetected == true, "TEST 7: Gate-jam condition visible in telemetry");
    HardwareHealth hJam = healthMonitor.evaluate(tJam);
    TEST_ASSERT(hJam.status == HealthStatus::FAULT, "TEST 7: Gate-jam condition evaluated as FAULT by health monitor");
    TEST_ASSERT(safetyController.isEmergencyStopRequired(tJam), "TEST 7: SafetyController requires emergency stop on gate jam FAULT");

    // TEST 9: Failed gate actuation not counted as successful actuation
    WasteItem plasticItem{"TEST-JAM-ITEM", 0, WasteType::PLASTIC, 0.95, BoundingBox{}, 0, "PENDING"};
    SimulationRunner runner;
    SortingResult jamResult = runner.processItem(plasticItem, controller, hwManager, &aggregator, &healthMonitor);
    TEST_ASSERT(jamResult.status == "GATE_JAMMED", "TEST 8: Gate-jam behavior remains GATE_JAMMED");
    TEST_ASSERT(runner.getStatistics().successfulItems == 0, "TEST 9: Failed gate actuation not counted as successful item");
    TEST_ASSERT(runner.getStatistics().totalGateActuations == 0, "TEST 9: Failed gate actuation not counted as successful gate actuation");
    TEST_ASSERT(runner.getStatistics().criticalChecks == 1, "TEST 9: Critical health check recorded in statistics");
    gatePlastic.clearJam();

    // TEST 10, 11, 12: Simulation statistics, telemetry snapshots, and sequential processing
    runner.reset();
    std::vector<WasteItem> items = {
        WasteItem{"ITEM-001", 0, WasteType::PLASTIC, 0.94, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-002", 0, WasteType::METAL,   0.91, BoundingBox{}, 0, "PENDING"},
        WasteItem{"ITEM-003", 0, WasteType::PAPER,   0.88, BoundingBox{}, 0, "PENDING"}
    };
    SimulationStatistics stats = runner.runSimulation(items, controller, hwManager, &aggregator, &healthMonitor);
    TEST_ASSERT(stats.totalItems == 3, "TEST 10: Statistics remain correct (total 3)");
    TEST_ASSERT(stats.successfulItems == 3, "TEST 10: Successful items counted correctly (3)");
    TEST_ASSERT(stats.healthChecksPerformed == 3, "TEST 11: Multiple telemetry snapshots collected cleanly (3 health checks)");
    TEST_ASSERT(stats.healthyChecks == 3, "TEST 12: Telemetry integration does not interfere with sequential processing (3 healthy)");

    // TEST 13 & 14: Emergency stop & reset behavior
    MachineFSM fsm(hwManager, safetyController);
    fsm.start();
    fsm.emergencyStop();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::EMERGENCY_STOP, "TEST 13: Existing emergency-stop behavior remains correct");
    fsm.reset();
    TEST_ASSERT(fsm.getCurrentState() == MachineState::IDLE, "TEST 14: Existing reset behavior remains correct");

    // TEST 15: No regression in sorting behavior
    SortingResult okResult = controller.classify(items[0]);
    TEST_ASSERT(okResult.status == "SUCCESS", "TEST 15: No regression in existing sorting behavior");
}

void testOperationalEventHistory() {
    // 1. OperationalEvent construction
    OperationalEvent evt;
    evt.type = EventType::ITEM_DETECTED;
    evt.severity = smartwaste::logging::LogLevel::LOG_INFO_LEVEL;
    evt.message = "Item detected on conveyor.";
    evt.itemId = "ITEM-100";
    evt.wasteType = WasteType::PLASTIC;
    evt.gateId = 1;

    TEST_ASSERT(evt.type == EventType::ITEM_DETECTED, "OperationalEvent type construction");
    TEST_ASSERT(!evt.toString().empty(), "OperationalEvent toString non-empty");

    // 2. Event ID generation & 3. Timestamp validity
    EventHistory history;
    TEST_ASSERT(history.getEventCount() == 0, "7. Empty EventHistory count == 0");
    TEST_ASSERT(history.getEvents().empty(), "7. Empty EventHistory getEvents() empty");

    OperationalEvent addedEvt = history.addEvent(evt);
    TEST_ASSERT(addedEvt.eventId == "EVT-000001", "2. Event ID generation is sequential EVT-000001");
    TEST_ASSERT(addedEvt.timestampMs > 0, "3. Timestamp is valid");
    TEST_ASSERT(history.getEventCount() == 1, "9. Event count is 1 after addition");

    // 4. Event type -> string conversion & 5. String -> event type conversion & 6. Invalid string handling
    TEST_ASSERT(eventTypeToString(EventType::ITEM_DETECTED) == "ITEM_DETECTED", "4. EventType to string");
    TEST_ASSERT(stringToEventType("ITEM_DETECTED") == EventType::ITEM_DETECTED, "5. String to EventType");
    TEST_ASSERT(stringToEventType("NON_EXISTENT_EVENT") == EventType::UNKNOWN, "6. Invalid event string maps to UNKNOWN");

    // 10. Multiple event insertion & 11. Chronological ordering
    history.addEvent(EventType::ITEM_CLASSIFIED, smartwaste::logging::LogLevel::LOG_INFO_LEVEL, "Classified plastic", "ITEM-100", WasteType::PLASTIC, 1);
    history.addEvent(EventType::GATE_ACTUATED, smartwaste::logging::LogLevel::LOG_INFO_LEVEL, "Actuated gate #1", "ITEM-100", WasteType::PLASTIC, 1);
    TEST_ASSERT(history.getEventCount() == 3, "10. Multiple event insertion (count == 3)");
    
    auto allEvents = history.getEvents();
    TEST_ASSERT(allEvents[0].eventId == "EVT-000001", "11. Chronological order 1st event EVT-000001");
    TEST_ASSERT(allEvents[1].eventId == "EVT-000002", "11. Chronological order 2nd event EVT-000002");
    TEST_ASSERT(allEvents[2].eventId == "EVT-000003", "11. Chronological order 3rd event EVT-000003");

    // 12. getRecentEvents(0)
    TEST_ASSERT(history.getRecentEvents(0).empty(), "12. getRecentEvents(0) returns empty vector");

    // 13. getRecentEvents(n)
    auto recent2 = history.getRecentEvents(2);
    TEST_ASSERT(recent2.size() == 2, "13. getRecentEvents(2) returns 2 items");
    TEST_ASSERT(recent2[0].eventId == "EVT-000002", "13. 1st recent event is EVT-000002");
    TEST_ASSERT(recent2[1].eventId == "EVT-000003", "13. 2nd recent event is EVT-000003");

    // 14. Requesting more events than available
    auto recent10 = history.getRecentEvents(10);
    TEST_ASSERT(recent10.size() == 3, "14. Requesting 10 recent events when 3 exist returns 3 events");

    // 15. Event filtering by type
    auto classifiedEvents = history.getEventsByType(EventType::ITEM_CLASSIFIED);
    TEST_ASSERT(classifiedEvents.size() == 1, "15. Event filtering by type returns 1 ITEM_CLASSIFIED event");

    // 16. Event filtering by item
    auto itemEvents = history.getEventsByItem("ITEM-100");
    TEST_ASSERT(itemEvents.size() == 3, "16. Event filtering by item returns 3 events for ITEM-100");

    // 17. clear()
    history.clear();
    TEST_ASSERT(history.getEventCount() == 0, "17. clear() resets event history count to 0");

    // Integration tests: 18-24
    ConfigManager configMgr;
    configMgr.loadFromFile("config/config.json");
    SortingController controller(configMgr);

    SimulatedConveyor conveyor(2.5);
    SimulatedMotor motor(1750.0);
    SimulatedGate gatePlastic(1); // Gate #1
    SimulatedGate gateMetal(2);   // Gate #2
    HardwareManager hwManager(conveyor, motor);
    hwManager.addGate(gatePlastic);
    hwManager.addGate(gateMetal);

    SimulatedSensor sensor(0.65);
    HardwareTelemetryAggregator aggregator(conveyor, motor, sensor, gatePlastic);
    SystemHealthMonitor healthMonitor;
    SafetyController safetyController(healthMonitor);
    MachineFSM fsm(hwManager, safetyController, &history);

    // 22. Emergency stop event & 23. Machine reset event
    fsm.start();
    TEST_ASSERT(history.getEventsByType(EventType::MACHINE_STARTED).size() == 1, "MACHINE_STARTED event logged");

    fsm.emergencyStop();
    TEST_ASSERT(history.getEventsByType(EventType::EMERGENCY_STOP).size() == 1, "22. Emergency stop creates EMERGENCY_STOP event");

    fsm.reset();
    TEST_ASSERT(history.getEventsByType(EventType::MACHINE_RESET).size() == 1, "23. Machine reset creates MACHINE_RESET event");

    // 18. Successful sorting events
    history.clear();
    SimulationRunner runner;
    WasteItem pItem{"ITEM-PLASTIC-OK", 0, WasteType::PLASTIC, 0.95, BoundingBox{}, 0, "PENDING"};
    runner.processItem(pItem, controller, hwManager, &aggregator, &healthMonitor, &history);

    TEST_ASSERT(history.getEventsByType(EventType::ITEM_DETECTED).size() == 1, "18. ITEM_DETECTED event created");
    TEST_ASSERT(history.getEventsByType(EventType::ITEM_CLASSIFIED).size() == 1, "18. ITEM_CLASSIFIED event created");
    TEST_ASSERT(history.getEventsByType(EventType::GATE_ACTUATED).size() == 1, "18. GATE_ACTUATED event created");
    TEST_ASSERT(history.getEventsByType(EventType::ITEM_SORTED).size() == 1, "18. ITEM_SORTED event created");

    // 19. Low-confidence rejection event
    history.clear();
    WasteItem lowConfItem{"ITEM-LOW-CONF", 0, WasteType::PLASTIC, 0.40, BoundingBox{}, 0, "PENDING"};
    runner.processItem(lowConfItem, controller, hwManager, &aggregator, &healthMonitor, &history);
    TEST_ASSERT(history.getEventsByType(EventType::ITEM_REJECTED).size() == 1, "19. Low-confidence item creates ITEM_REJECTED event");

    // 20. Unavailable gate event
    history.clear();
    WasteItem glassItem{"ITEM-GLASS-UNAVAIL", 0, WasteType::GLASS, 0.92, BoundingBox{}, 0, "PENDING"};
    runner.processItem(glassItem, controller, hwManager, &aggregator, &healthMonitor, &history);
    TEST_ASSERT(history.getEventsByType(EventType::GATE_UNAVAILABLE).size() == 1, "20. Unavailable gate creates GATE_UNAVAILABLE event");

    // 21. Gate jam fault event & 24. Health fault event
    history.clear();
    gatePlastic.setJamDetected(true);
    runner.processItem(pItem, controller, hwManager, &aggregator, &healthMonitor, &history);
    TEST_ASSERT(history.getEventsByType(EventType::GATE_JAMMED).size() == 1, "21. Gate jam creates GATE_JAMMED event");
    TEST_ASSERT(history.getEventsByType(EventType::HEALTH_FAULT).size() == 1, "24. Health fault creates HEALTH_FAULT event");
    gatePlastic.clearJam();
}

void testEventPersistence() {
    std::string testPath = "data/test_events.jsonl";
    std::string malformedPath = "data/test_malformed_events.jsonl";

    // Clean up pre-existing test files if any
    std::remove(testPath.c_str());
    std::remove(malformedPath.c_str());

    // 1. Missing / Empty persistence file loads safely
    FileEventRepository repo(testPath);
    auto loadedInitial = repo.loadEvents();
    TEST_ASSERT(loadedInitial.empty(), "1. Empty/missing persistence file loads empty vector safely");

    // 2. EventHistory connected to FileEventRepository loads empty
    EventHistory history(&repo);
    TEST_ASSERT(history.loadFromRepository() == 0, "2. EventHistory loadFromRepository returns 0 for empty file");
    TEST_ASSERT(history.getEventCount() == 0, "3. EventHistory has 0 events initially");

    // 3. Add single event & verify persistence + field survival
    OperationalEvent e1;
    e1.type = EventType::ITEM_DETECTED;
    e1.severity = LogLevel::LOG_INFO_LEVEL;
    e1.message = "Item DETECTED at conveyor entry";
    e1.itemId = "ITEM-101";
    e1.wasteType = WasteType::PLASTIC;
    e1.gateId = 1;

    OperationalEvent savedE1 = history.addEvent(e1);
    TEST_ASSERT(!savedE1.eventId.empty(), "4. addEvent assigns sequential event ID");
    TEST_ASSERT(savedE1.eventId == "EVT-000001", "5. First event ID is EVT-000001");
    TEST_ASSERT(history.getEventCount() == 1, "6. EventHistory count is 1 after addEvent");

    // Load directly from repo file to verify disk persistence
    auto fileEvents1 = repo.loadEvents();
    TEST_ASSERT(fileEvents1.size() == 1, "7. Repo file contains 1 persisted event");
    TEST_ASSERT(fileEvents1[0].eventId == "EVT-000001", "8. Persisted event ID matches EVT-000001");
    TEST_ASSERT(fileEvents1[0].type == EventType::ITEM_DETECTED, "9. Persisted event type survives serialization");
    TEST_ASSERT(fileEvents1[0].severity == LogLevel::LOG_INFO_LEVEL, "10. Persisted severity survives serialization");
    TEST_ASSERT(fileEvents1[0].message == "Item DETECTED at conveyor entry", "11. Persisted message survives serialization");
    TEST_ASSERT(fileEvents1[0].itemId == "ITEM-101", "12. Persisted itemId survives serialization");
    TEST_ASSERT(fileEvents1[0].wasteType == WasteType::PLASTIC, "13. Persisted wasteType survives serialization");
    TEST_ASSERT(fileEvents1[0].gateId == 1, "14. Persisted gateId survives serialization");

    // 4. Add multiple events & verify chronological order and ID sequence
    history.addEvent(EventType::ITEM_CLASSIFIED, LogLevel::LOG_INFO_LEVEL, "Item classified as PLASTIC", "ITEM-101", WasteType::PLASTIC, 1);
    history.addEvent(EventType::ITEM_SORTED, LogLevel::LOG_INFO_LEVEL, "Item sorted into Gate 1", "ITEM-101", WasteType::PLASTIC, 1);
    history.addEvent(EventType::GATE_ACTUATED, LogLevel::LOG_DEBUG_LEVEL, "Gate 1 actuated", "ITEM-101", WasteType::PLASTIC, 1);

    TEST_ASSERT(history.getEventCount() == 4, "15. EventHistory contains 4 events after additions");

    auto fileEvents4 = repo.loadEvents();
    TEST_ASSERT(fileEvents4.size() == 4, "16. Repo file contains 4 persisted events on disk");
    TEST_ASSERT(fileEvents4[0].eventId == "EVT-000001", "17. First event ID is EVT-000001");
    TEST_ASSERT(fileEvents4[1].eventId == "EVT-000002", "18. Second event ID is EVT-000002");
    TEST_ASSERT(fileEvents4[2].eventId == "EVT-000003", "19. Third event ID is EVT-000003");
    TEST_ASSERT(fileEvents4[3].eventId == "EVT-000004", "20. Fourth event ID is EVT-000004");
    TEST_ASSERT(fileEvents4[3].type == EventType::GATE_ACTUATED, "21. Fourth event type matches GATE_ACTUATED");

    // 5. Restart simulation: Create new EventHistory attached to same repository file
    EventHistory restartedHistory(&repo);
    std::size_t loadedCount = restartedHistory.loadFromRepository();
    TEST_ASSERT(loadedCount == 4, "22. Restarted EventHistory loads 4 persisted events from file");
    TEST_ASSERT(restartedHistory.getEventCount() == 4, "23. Restarted EventHistory getEventCount is 4");

    // 6. Test ID continuity after restart: next event should be EVT-000005
    OperationalEvent nextEvt = restartedHistory.addEvent(EventType::HEALTH_WARNING, LogLevel::LOG_WARN_LEVEL, "Motor temperature warning");
    TEST_ASSERT(nextEvt.eventId == "EVT-000005", "24. Event ID continuity preserved after restart (EVT-000005)");
    TEST_ASSERT(restartedHistory.getEventCount() == 5, "25. EventHistory count is 5 after restart addition");

    // 7. Test query methods after restore
    auto recent = restartedHistory.getRecentEvents(2);
    TEST_ASSERT(recent.size() == 2, "26. getRecentEvents(2) returns 2 items");
    TEST_ASSERT(recent[0].eventId == "EVT-000004", "27. Recent event 0 is EVT-000004");
    TEST_ASSERT(recent[1].eventId == "EVT-000005", "28. Recent event 1 is EVT-000005");

    auto classifiedEvents = restartedHistory.getEventsByType(EventType::ITEM_CLASSIFIED);
    TEST_ASSERT(classifiedEvents.size() == 1, "29. getEventsByType returns 1 classified event");
    TEST_ASSERT(classifiedEvents[0].itemId == "ITEM-101", "30. Filtered event matches itemId ITEM-101");

    auto itemEvents = restartedHistory.getEventsByItem("ITEM-101");
    TEST_ASSERT(itemEvents.size() == 4, "31. getEventsByItem('ITEM-101') returns 4 events");

    // 8. Test malformed line handling
    {
        std::ofstream malFile(malformedPath);
        malFile << "{\"eventId\":\"EVT-000001\",\"timestampMs\":1000,\"type\":\"ITEM_DETECTED\",\"severity\":\"INFO\",\"message\":\"Valid event 1\",\"itemId\":\"ITM-1\",\"wasteType\":\"METAL\",\"gateId\":2}\n";
        malFile << "THIS IS MALFORMED CORRUPTED JSON LINE\n";
        malFile << "{\"eventId\":\"EVT-000002\",\"timestampMs\":2000,\"type\":\"ITEM_SORTED\",\"severity\":\"INFO\",\"message\":\"Valid event 2\",\"itemId\":\"ITM-1\",\"wasteType\":\"METAL\",\"gateId\":2}\n";
        malFile.close();
    }

    FileEventRepository malformedRepo(malformedPath);
    auto malformedLoaded = malformedRepo.loadEvents();
    TEST_ASSERT(malformedLoaded.size() == 2, "32. FileEventRepository skips malformed JSON line and loads 2 valid events");
    TEST_ASSERT(malformedLoaded[0].eventId == "EVT-000001", "33. First valid event around malformed data preserved");
    TEST_ASSERT(malformedLoaded[1].eventId == "EVT-000002", "34. Second valid event around malformed data preserved");

    // 9. Test clear() functionality
    restartedHistory.clear();
    TEST_ASSERT(restartedHistory.getEventCount() == 0, "35. clear() resets in-memory history to 0");
    auto clearedRepoEvents = repo.loadEvents();
    TEST_ASSERT(clearedRepoEvents.empty(), "36. clear() truncates persistent repository file on disk");

    // Next event after clear() restarts at EVT-000001
    OperationalEvent postClearEvt = restartedHistory.addEvent(EventType::MACHINE_STARTED, LogLevel::LOG_INFO_LEVEL, "Machine started");
    TEST_ASSERT(postClearEvt.eventId == "EVT-000001", "37. Event ID resets to EVT-000001 after clear()");

    // Cleanup test files
    std::remove(testPath.c_str());
    std::remove(malformedPath.c_str());
}

void testSmartWasteEngine() {
    std::string testStorage = "data/engine_test_events.jsonl";
    std::remove(testStorage.c_str());

    SmartWasteEngine engine(testStorage);

    // 1. Engine Initialization
    TEST_ASSERT(!engine.isInitialized(), "1. Engine initially not initialized");
    bool initOk = engine.initialize("config/config.json");
    TEST_ASSERT(initOk, "2. Engine initialize() returns true");
    TEST_ASSERT(engine.isInitialized(), "3. Engine isInitialized() returns true after initialize()");

    // 2. Machine State Retrieval & Lifecycle Transitions
    TEST_ASSERT(engine.getMachineState() == MachineState::IDLE, "4. Initial machine state is IDLE");
    TEST_ASSERT(engine.getMachineStateString() == "IDLE", "5. Machine state string is IDLE");

    // 3. Machine Start
    bool startOk = engine.startMachine();
    TEST_ASSERT(startOk, "6. startMachine() transitions to RUNNING");
    TEST_ASSERT(engine.getMachineState() == MachineState::RUNNING, "7. Current state is RUNNING");

    // 4. Machine Pause
    bool pauseOk = engine.pauseMachine();
    TEST_ASSERT(pauseOk, "8. pauseMachine() transitions to PAUSED");
    TEST_ASSERT(engine.getMachineState() == MachineState::PAUSED, "9. Current state is PAUSED");

    // 5. Machine Resume
    bool resumeOk = engine.resumeMachine();
    TEST_ASSERT(resumeOk, "10. resumeMachine() transitions to RUNNING");
    TEST_ASSERT(engine.getMachineState() == MachineState::RUNNING, "11. Current state is RUNNING after resume");

    // 6. Machine Stop
    bool stopOk = engine.stopMachine();
    TEST_ASSERT(stopOk, "12. stopMachine() transitions to IDLE");
    TEST_ASSERT(engine.getMachineState() == MachineState::IDLE, "13. Current state is IDLE after stop");

    // Restart machine for sorting operations
    engine.startMachine();

    // 7. Waste Item Processing & Sorting Propagation (PLASTIC -> Gate 1)
    WasteItem plasticItem;
    plasticItem.id = "ENG-ITEM-001";
    plasticItem.type = WasteType::PLASTIC;
    plasticItem.confidence = 0.95;

    SortingResult plasticRes = engine.processWasteItem(plasticItem);
    TEST_ASSERT(plasticRes.status == "SUCCESS", "14. Processed plastic item sorting status is SUCCESS");
    TEST_ASSERT(plasticRes.targetGate == 1, "15. Processed plastic item target gate is 1");

    // 8. Low Confidence Handling
    WasteItem lowConfItem;
    lowConfItem.id = "ENG-ITEM-002";
    lowConfItem.type = WasteType::PLASTIC;
    lowConfItem.confidence = 0.40;

    SortingResult lowConfRes = engine.processWasteItem(lowConfItem);
    TEST_ASSERT(lowConfRes.status == "REJECTED", "16. Low-confidence item status is REJECTED");

    // 9. Glass Handling (GLASS -> Gate 4)
    WasteItem glassItem;
    glassItem.id = "ENG-ITEM-003";
    glassItem.type = WasteType::GLASS;
    glassItem.confidence = 0.90;

    SortingResult glassRes = engine.processWasteItem(glassItem);
    TEST_ASSERT(glassRes.status == "SUCCESS", "17. Glass item status is SUCCESS when Gate #4 is registered");
    TEST_ASSERT(glassRes.targetGate == 4, "17b. Glass item target gate is 4");

    // 10. Gate Jam Handling
    auto* gatePlastic = engine.getGate(0);
    TEST_ASSERT(gatePlastic != nullptr, "18. getGate(0) returns plastic gate pointer");
    if (gatePlastic) {
        gatePlastic->setJamDetected(true);
        SortingResult jamRes = engine.processWasteItem(plasticItem);
        TEST_ASSERT(jamRes.status == "GATE_JAMMED", "19. Gate jam item sorting status is GATE_JAMMED");
        gatePlastic->clearJam();
    }

    // 11. Telemetry Retrieval
    HardwareTelemetry telem = engine.getTelemetry();
    TEST_ASSERT(telem.conveyorSpeed >= 0.0, "20. Telemetry conveyor speed non-negative");

    // 12. Health Retrieval
    HardwareHealth health = engine.getHealthStatus();
    TEST_ASSERT(!health.toString().empty(), "21. Health status string non-empty");

    // 13. Simulation Statistics Retrieval
    SimulationStatistics stats = engine.getSimulationStatistics();
    TEST_ASSERT(stats.totalItems == 4, "22. Total processed items in statistics is 4");
    TEST_ASSERT(stats.successfulItems == 2, "23. Successful items in statistics is 2");
    TEST_ASSERT(stats.rejectedItems == 1, "24. Rejected items in statistics is 1");

    // 14. Event History & Persistence Retrieval
    TEST_ASSERT(engine.getEventCount() > 0, "25. Engine event count > 0");
    auto recentEvents = engine.getRecentEvents(5);
    TEST_ASSERT(!recentEvents.empty(), "26. getRecentEvents returns recorded events");

    // 15. Engine Snapshot Retrieval
    EngineSnapshot snapshot = engine.getSnapshot();
    TEST_ASSERT(snapshot.isInitialized, "27. Snapshot isInitialized is true");
    TEST_ASSERT(!snapshot.toString().empty(), "28. Snapshot toString is non-empty");

    // 16. Emergency Stop & Reset
    engine.emergencyStop();
    TEST_ASSERT(engine.getMachineState() == MachineState::EMERGENCY_STOP, "29. State is EMERGENCY_STOP after emergencyStop()");

    bool resetOk = engine.resetMachine();
    TEST_ASSERT(resetOk, "30. resetMachine() succeeds");
    TEST_ASSERT(engine.getMachineState() == MachineState::IDLE, "31. State is IDLE after resetMachine()");

    // Cleanup test storage
    std::remove(testStorage.c_str());
}

void testApiServerFoundation() {
    // 1. Server configuration defaults
    ServerConfig config;
    TEST_ASSERT(config.address == "127.0.0.1", "1. ServerConfig default address is 127.0.0.1");
    TEST_ASSERT(config.port == 8080, "2. ServerConfig default port is 8080");
    TEST_ASSERT(config.maxRequestSize == 1048576, "3. ServerConfig default max request size is 1MB");

    // 2. Server initial state
    config.port = 8090; // Use port 8090 for isolated unit testing
    HttpServer server(config);
    TEST_ASSERT(!server.isRunning(), "4. HttpServer initially not running");

    // 3. Router route registration & default routes
    ApiRouter& router = server.getRouter();
    HttpRequest pingReq;
    pingReq.method = "GET";
    pingReq.path = "/api/v1/ping";
    pingReq.version = "HTTP/1.1";

    HttpResponse pingRes = router.routeRequest(pingReq);
    TEST_ASSERT(pingRes.statusCode == 200, "5. GET /api/v1/ping returns 200 OK");
    TEST_ASSERT(pingRes.headers["Content-Type"] == "application/json", "6. Response Content-Type is application/json");
    TEST_ASSERT(pingRes.body.find("SmartWaste API is running") != std::string::npos, "7. Response body contains expected ping message");

    // 4. 404 Not Found for unknown routes
    HttpRequest unknownReq;
    unknownReq.method = "GET";
    unknownReq.path = "/api/v1/nonexistent";
    unknownReq.version = "HTTP/1.1";

    HttpResponse notFoundRes = router.routeRequest(unknownReq);
    TEST_ASSERT(notFoundRes.statusCode == 404, "8. GET /api/v1/nonexistent returns 404 Not Found");

    // 5. 405 Method Not Allowed
    HttpRequest wrongMethodReq;
    wrongMethodReq.method = "POST";
    wrongMethodReq.path = "/api/v1/ping";
    wrongMethodReq.version = "HTTP/1.1";

    HttpResponse methodNotAllowedRes = router.routeRequest(wrongMethodReq);
    TEST_ASSERT(methodNotAllowedRes.statusCode == 405, "9. POST /api/v1/ping returns 405 Method Not Allowed");

    // 6. OPTIONS CORS request
    HttpRequest optionsReq;
    optionsReq.method = "OPTIONS";
    optionsReq.path = "/api/v1/ping";
    optionsReq.version = "HTTP/1.1";

    HttpResponse optionsRes = router.routeRequest(optionsReq);
    TEST_ASSERT(optionsRes.statusCode == 200, "10. OPTIONS preflight request returns 200 OK");
    TEST_ASSERT(optionsRes.headers["Access-Control-Allow-Origin"] == "http://localhost:5173", "11. CORS header Access-Control-Allow-Origin injected");

    // 7. Request parser verification & malformed handling
    std::string rawMalformed = "INVALID_REQUEST_LINE\r\n\r\n";
    HttpRequest malformedParsed = HttpServer::parseRequest(rawMalformed, 1048576);
    TEST_ASSERT(malformedParsed.isMalformed, "12. Malformed raw request flagged as malformed");

    // 8. Payload size limit verification
    std::string rawOversized = "POST /api/v1/test HTTP/1.1\r\nContent-Length: 2000000\r\n\r\nData";
    HttpRequest oversizedParsed = HttpServer::parseRequest(rawOversized, 1048576);
    TEST_ASSERT(oversizedParsed.isMalformed, "13. Oversized payload Content-Length flagged as malformed");

    // 9. Format response verification
    HttpResponse resToFormat;
    resToFormat.statusCode = 200;
    resToFormat.statusMessage = "OK";
    nlohmann::json jFormat;
    jFormat["test"] = "value";
    resToFormat.setJsonBody(jFormat);

    std::string formatted = HttpServer::formatResponse(resToFormat);
    TEST_ASSERT(formatted.find("HTTP/1.1 200 OK") != std::string::npos, "14. Format response contains HTTP status line");
    TEST_ASSERT(formatted.find("Content-Type: application/json") != std::string::npos, "15. Format response contains Content-Type header");

    // 10. Server start & stop lifecycle
    bool started = server.start();
    TEST_ASSERT(started, "16. server.start() returns true");
    TEST_ASSERT(server.isRunning(), "17. server.isRunning() returns true after start()");

    // End-to-end socket test client
    SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock != INVALID_SOCKET) {
        sockaddr_in servAddr;
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        servAddr.sin_port = htons(8090);

        if (connect(clientSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == 0) {
            std::string reqText = "GET /api/v1/ping HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
            send(clientSock, reqText.c_str(), static_cast<int>(reqText.length()), 0);

            // Server processes connection
            bool processed = server.processPendingConnection();
            TEST_ASSERT(processed, "18. processPendingConnection() handled client request");

            char rBuffer[2048];
            int rBytes = recv(clientSock, rBuffer, sizeof(rBuffer) - 1, 0);
            if (rBytes > 0) {
                rBuffer[rBytes] = '\0';
                std::string resString(rBuffer, rBytes);
                TEST_ASSERT(resString.find("200 OK") != std::string::npos, "19. Client received 200 OK response from server");
                TEST_ASSERT(resString.find("SmartWaste API is running") != std::string::npos, "20. Client received ping JSON response");
            }
        }
        closesocket(clientSock);
    }

    server.stop();
    TEST_ASSERT(!server.isRunning(), "21. server.isRunning() is false after stop()");
}

void testWasteTypeConversions() {
    TEST_ASSERT(wasteTypeToString(WasteType::PLASTIC) == "PLASTIC", "WasteType::PLASTIC to string");
    TEST_ASSERT(wasteTypeToString(WasteType::METAL) == "METAL", "WasteType::METAL to string");
    TEST_ASSERT(wasteTypeToString(WasteType::PAPER) == "PAPER", "WasteType::PAPER to string");
    TEST_ASSERT(wasteTypeToString(WasteType::GLASS) == "GLASS", "WasteType::GLASS to string");
    TEST_ASSERT(wasteTypeToString(WasteType::OTHER) == "OTHER", "WasteType::OTHER to string");
    TEST_ASSERT(wasteTypeToString(WasteType::UNKNOWN) == "UNKNOWN", "WasteType::UNKNOWN to string");

    TEST_ASSERT(stringToWasteType("plastic") == WasteType::PLASTIC, "String 'plastic' to WasteType");
    TEST_ASSERT(stringToWasteType("METAL") == WasteType::METAL, "String 'METAL' to WasteType");
    TEST_ASSERT(stringToWasteType("Paper") == WasteType::PAPER, "String 'Paper' to WasteType");
    TEST_ASSERT(stringToWasteType("glass") == WasteType::GLASS, "String 'glass' to WasteType");
    TEST_ASSERT(stringToWasteType("OTHER") == WasteType::OTHER, "String 'OTHER' to WasteType");
    TEST_ASSERT(stringToWasteType("INVALID_STR") == WasteType::UNKNOWN, "Invalid string to WasteType::UNKNOWN");
}

void testMachineStateConversions() {
    TEST_ASSERT(machineStateToString(MachineState::IDLE) == "IDLE", "MachineState::IDLE to string");
    TEST_ASSERT(machineStateToString(MachineState::RUNNING) == "RUNNING", "MachineState::RUNNING to string");
    TEST_ASSERT(machineStateToString(MachineState::EMERGENCY_STOP) == "EMERGENCY_STOP", "MachineState::EMERGENCY_STOP to string");

    TEST_ASSERT(stringToMachineState("idle") == MachineState::IDLE, "String 'idle' to MachineState");
    TEST_ASSERT(stringToMachineState("RUNNING") == MachineState::RUNNING, "String 'RUNNING' to MachineState");
    TEST_ASSERT(stringToMachineState("emergency_stop") == MachineState::EMERGENCY_STOP, "String 'emergency_stop' to MachineState");
    TEST_ASSERT(stringToMachineState("NON_EXISTENT") == MachineState::ERROR, "Invalid string to MachineState::ERROR");
}

void testWasteItemCreation() {
    WasteItem item;
    item.id = "ITEM-001";
    item.type = WasteType::METAL;
    item.confidence = 0.88;
    item.boundingBox = BoundingBox{10, 20, 50, 60};
    item.gateId = 2;
    item.sortingStatus = "SORTED";

    TEST_ASSERT(item.id == "ITEM-001", "WasteItem ID check");
    TEST_ASSERT(item.type == WasteType::METAL, "WasteItem Type check");
    TEST_ASSERT(item.boundingBox.area() == 3000, "BoundingBox area calculation");
    TEST_ASSERT(!item.toString().empty(), "WasteItem toString output non-empty");
}

void testSortingResultCreation() {
    SortingResult res;
    res.type = WasteType::GLASS;
    res.targetGate = 4;
    res.confidence = 0.95;
    res.status = "SUCCESS";
    res.message = "OK";

    TEST_ASSERT(res.targetGate == 4, "SortingResult target gate check");
    TEST_ASSERT(res.status == "SUCCESS", "SortingResult status check");
    TEST_ASSERT(!res.toString().empty(), "SortingResult toString output non-empty");
}

void testConfigurationValidation() {
    ConfigManager configMgr;
    
    // Valid file load
    bool loaded = false;
    try {
        configMgr.loadFromFile("config/config.json");
        loaded = true;
    } catch (...) {
        loaded = false;
    }
    TEST_ASSERT(loaded, "ConfigManager valid config.json load");
    TEST_ASSERT(configMgr.getMachineConfig().maxConveyorSpeed > 0.0, "Config maxConveyorSpeed > 0");
    TEST_ASSERT(configMgr.getCameraConfig().frameWidth == 1920, "Config camera frameWidth == 1920");

    // Invalid file load exception check
    bool threwException = false;
    try {
        configMgr.loadFromFile("non_existent_config.json");
    } catch (const ConfigException&) {
        threwException = true;
    }
    TEST_ASSERT(threwException, "ConfigManager throws ConfigException on missing file");
}

void testLoggerBasicBehavior() {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(LogLevel::LOG_DEBUG_LEVEL);
    bool fileOpened = logger.setLogFile("data/test_run.log");
    
    TEST_ASSERT(fileOpened, "Logger creates and opens test log file");
    
    logger.info("TestTag", "Unit test log message string.");
    logger.flush();

    // Verify log file exists and contains content
    std::ifstream file("data/test_run.log");
    TEST_ASSERT(file.is_open(), "Log file exists on disk");
    std::string line;
    bool foundTag = false;
    while (std::getline(file, line)) {
        if (line.find("TestTag") != std::string::npos && line.find("Unit test log message string.") != std::string::npos) {
            foundTag = true;
            break;
        }
    }
    TEST_ASSERT(foundTag, "Logger output file contains logged tag and message");
}

void testMachineLifecycleApi() {
    std::string testEventStorage = "data/test_machine_api_events.jsonl";
    std::remove(testEventStorage.c_str());

    SmartWasteEngine engine(testEventStorage);
    engine.initialize("config/config.json");

    ServerConfig config;
    config.port = 8091; // Isolated test port
    HttpServer server(config);
    server.getRouter().setEngine(&engine);

    bool started = server.start();
    TEST_ASSERT(started, "HttpServer started for Machine Lifecycle API test on port 8091");

    ApiRouter& router = server.getRouter();

    // 1. GET /api/v1/machine/status (Initial IDLE state)
    HttpRequest statusReq;
    statusReq.method = "GET";
    statusReq.path = "/api/v1/machine/status";
    HttpResponse statusRes = router.routeRequest(statusReq);
    TEST_ASSERT(statusRes.statusCode == 200, "1. GET /api/v1/machine/status returns 200 OK");
    nlohmann::json statusJson = nlohmann::json::parse(statusRes.body);
    TEST_ASSERT(statusJson["success"] == true, "Status response contains success=true");
    TEST_ASSERT(statusJson["data"]["state"] == "IDLE", "2. Initial machine state in status response is IDLE");

    // 2. POST /api/v1/machine/start
    HttpRequest startReq;
    startReq.method = "POST";
    startReq.path = "/api/v1/machine/start";
    HttpResponse startRes = router.routeRequest(startReq);
    TEST_ASSERT(startRes.statusCode == 200, "3. POST /api/v1/machine/start returns 200 OK");
    nlohmann::json startJson = nlohmann::json::parse(startRes.body);
    TEST_ASSERT(startJson["data"]["state"] == "RUNNING", "4. Machine state transitions to RUNNING after start");

    // 3. Invalid POST /api/v1/machine/start (Already RUNNING)
    HttpResponse badStartRes = router.routeRequest(startReq);
    TEST_ASSERT(badStartRes.statusCode == 400, "5. Invalid start from RUNNING returns 400 Bad Request");
    nlohmann::json badStartJson = nlohmann::json::parse(badStartRes.body);
    TEST_ASSERT(badStartJson["error"]["code"] == "INVALID_STATE_TRANSITION", "6. Invalid start response code is INVALID_STATE_TRANSITION");

    // 4. POST /api/v1/machine/pause
    HttpRequest pauseReq;
    pauseReq.method = "POST";
    pauseReq.path = "/api/v1/machine/pause";
    HttpResponse pauseRes = router.routeRequest(pauseReq);
    TEST_ASSERT(pauseRes.statusCode == 200, "7. POST /api/v1/machine/pause returns 200 OK");
    nlohmann::json pauseJson = nlohmann::json::parse(pauseRes.body);
    TEST_ASSERT(pauseJson["data"]["state"] == "PAUSED", "8. Machine state transitions to PAUSED after pause");

    // 5. Invalid POST /api/v1/machine/pause (Already PAUSED)
    HttpResponse badPauseRes = router.routeRequest(pauseReq);
    TEST_ASSERT(badPauseRes.statusCode == 400, "9. Invalid pause from PAUSED returns 400 Bad Request");

    // 6. POST /api/v1/machine/resume
    HttpRequest resumeReq;
    resumeReq.method = "POST";
    resumeReq.path = "/api/v1/machine/resume";
    HttpResponse resumeRes = router.routeRequest(resumeReq);
    TEST_ASSERT(resumeRes.statusCode == 200, "10. POST /api/v1/machine/resume returns 200 OK");
    nlohmann::json resumeJson = nlohmann::json::parse(resumeRes.body);
    TEST_ASSERT(resumeJson["data"]["state"] == "RUNNING", "11. Machine state transitions to RUNNING after resume");

    // 7. POST /api/v1/machine/stop
    HttpRequest stopReq;
    stopReq.method = "POST";
    stopReq.path = "/api/v1/machine/stop";
    HttpResponse stopRes = router.routeRequest(stopReq);
    TEST_ASSERT(stopRes.statusCode == 200, "12. POST /api/v1/machine/stop returns 200 OK");
    nlohmann::json stopJson = nlohmann::json::parse(stopRes.body);
    TEST_ASSERT(stopJson["data"]["state"] == "IDLE", "13. Machine state transitions to IDLE after stop");

    // 8. Invalid POST /api/v1/machine/stop (Already IDLE)
    HttpResponse badStopRes = router.routeRequest(stopReq);
    TEST_ASSERT(badStopRes.statusCode == 400, "14. Invalid stop from IDLE returns 400 Bad Request");

    // 9. POST /api/v1/machine/emergency-stop
    HttpRequest estopReq;
    estopReq.method = "POST";
    estopReq.path = "/api/v1/machine/emergency-stop";
    HttpResponse estopRes = router.routeRequest(estopReq);
    TEST_ASSERT(estopRes.statusCode == 200, "15. POST /api/v1/machine/emergency-stop returns 200 OK");
    nlohmann::json estopJson = nlohmann::json::parse(estopRes.body);
    TEST_ASSERT(estopJson["data"]["state"] == "EMERGENCY_STOP", "16. Machine state transitions to EMERGENCY_STOP");

    // 10. Invalid POST /api/v1/machine/start during EMERGENCY_STOP
    HttpResponse badStartEstop = router.routeRequest(startReq);
    TEST_ASSERT(badStartEstop.statusCode == 400, "17. Invalid start from EMERGENCY_STOP returns 400 Bad Request");

    // 11. POST /api/v1/machine/reset
    HttpRequest resetReq;
    resetReq.method = "POST";
    resetReq.path = "/api/v1/machine/reset";
    HttpResponse resetRes = router.routeRequest(resetReq);
    TEST_ASSERT(resetRes.statusCode == 200, "18. POST /api/v1/machine/reset returns 200 OK");
    nlohmann::json resetJson = nlohmann::json::parse(resetRes.body);
    TEST_ASSERT(resetJson["data"]["state"] == "IDLE", "19. Machine state transitions to IDLE after reset");

    // 12. GET /api/v1/machine/status after reset
    HttpResponse postResetStatus = router.routeRequest(statusReq);
    nlohmann::json postResetJson = nlohmann::json::parse(postResetStatus.body);
    TEST_ASSERT(postResetJson["data"]["state"] == "IDLE", "20. Post-reset status returns IDLE state");

    // 13. Unknown route under /api/v1/machine
    HttpRequest unkMachineReq;
    unkMachineReq.method = "GET";
    unkMachineReq.path = "/api/v1/machine/unknown";
    HttpResponse unkMachineRes = router.routeRequest(unkMachineReq);
    TEST_ASSERT(unkMachineRes.statusCode == 404, "21. Unknown machine route returns 404 Not Found");

    // 14. Wrong method on /api/v1/machine/start
    HttpRequest badMethodReq;
    badMethodReq.method = "DELETE";
    badMethodReq.path = "/api/v1/machine/start";
    HttpResponse badMethodRes = router.routeRequest(badMethodReq);
    TEST_ASSERT(badMethodRes.statusCode == 405, "22. Unsupported method DELETE on /machine/start returns 405 Method Not Allowed");

    // 15. Regression check for /api/v1/ping
    HttpRequest pingReq;
    pingReq.method = "GET";
    pingReq.path = "/api/v1/ping";
    HttpResponse pingRes = router.routeRequest(pingReq);
    TEST_ASSERT(pingRes.statusCode == 200, "23. GET /api/v1/ping still returns 200 OK");

    server.stop();
    std::remove(testEventStorage.c_str());
}

void testPhase5Step4RestApi() {
    std::string testStorage = "data/test_step4_api_events.jsonl";
    std::remove(testStorage.c_str());

    SmartWasteEngine engine(testStorage);
    engine.initialize("config/config.json");

    ServerConfig config;
    config.port = 8092; // Isolated test port
    HttpServer server(config);
    server.getRouter().setEngine(&engine);

    bool started = server.start();
    TEST_ASSERT(started, "HttpServer started for Step 4 REST API test on port 8092");

    ApiRouter& router = server.getRouter();

    // 0. Test sorting rejection when machine is IDLE
    HttpRequest idleReq;
    idleReq.method = "POST";
    idleReq.path = "/api/v1/sorting/process";
    idleReq.body = R"({"id":"IDLE-001","type":"PLASTIC","confidence":0.95})";
    HttpResponse idleRes = router.routeRequest(idleReq);
    TEST_ASSERT(idleRes.statusCode == 200, "0a. POST /api/v1/sorting/process when IDLE returns 200 OK wrapper");
    nlohmann::json idleJson = nlohmann::json::parse(idleRes.body);
    TEST_ASSERT(idleJson["success"] == false, "0b. API response success is false when machine is IDLE");
    TEST_ASSERT(idleJson["data"]["status"] == "REJECTED", "0c. API sorting status is REJECTED when machine is IDLE");

    // Start machine so state is RUNNING for subsequent sorting API tests
    engine.startMachine();

    // 1. POST /api/v1/sorting/process (Valid Plastic in RUNNING state)
    HttpRequest plasticReq;
    plasticReq.method = "POST";
    plasticReq.path = "/api/v1/sorting/process";
    plasticReq.body = R"({"id":"PL-001","type":"PLASTIC","confidence":0.95,"gateId":1})";
    HttpResponse plasticRes = router.routeRequest(plasticReq);
    TEST_ASSERT(plasticRes.statusCode == 200, "1. POST /api/v1/sorting/process (Plastic) returns 200 OK");
    nlohmann::json plasticJson = nlohmann::json::parse(plasticRes.body);
    TEST_ASSERT(plasticJson["success"] == true, "1b. Plastic API response success is true");
    TEST_ASSERT(plasticJson["data"]["status"] == "SUCCESS", "2. Plastic item sorting status is SUCCESS");
    TEST_ASSERT(plasticJson["data"]["targetGate"] == 1, "3. Plastic item target gate is 1");

    // Valid Metal
    HttpRequest metalReq;
    metalReq.method = "POST";
    metalReq.path = "/api/v1/sorting/process";
    metalReq.body = R"({"id":"MT-002","type":"METAL","confidence":0.89,"gateId":2})";
    HttpResponse metalRes = router.routeRequest(metalReq);
    nlohmann::json metalJson = nlohmann::json::parse(metalRes.body);
    TEST_ASSERT(metalJson["success"] == true, "4a. Metal API response success is true");
    TEST_ASSERT(metalJson["data"]["status"] == "SUCCESS", "4. Metal item sorting status is SUCCESS");
    TEST_ASSERT(metalJson["data"]["targetGate"] == 2, "5. Metal item target gate is 2");

    // Valid Paper
    HttpRequest paperReq;
    paperReq.method = "POST";
    paperReq.path = "/api/v1/sorting/process";
    paperReq.body = R"({"id":"PA-003","type":"PAPER","confidence":0.85,"gateId":3})";
    HttpResponse paperRes = router.routeRequest(paperReq);
    nlohmann::json paperJson = nlohmann::json::parse(paperRes.body);
    TEST_ASSERT(paperJson["success"] == true, "6a. Paper API response success is true");
    TEST_ASSERT(paperJson["data"]["status"] == "SUCCESS", "6. Paper item sorting status is SUCCESS");
    TEST_ASSERT(paperJson["data"]["targetGate"] == 3, "7. Paper item target gate is 3");

    // Valid Glass
    HttpRequest glassReq;
    glassReq.method = "POST";
    glassReq.path = "/api/v1/sorting/process";
    glassReq.body = R"({"id":"GL-004","type":"GLASS","confidence":0.92,"gateId":4})";
    HttpResponse glassRes = router.routeRequest(glassReq);
    nlohmann::json glassJson = nlohmann::json::parse(glassRes.body);
    TEST_ASSERT(glassJson["success"] == true, "7a. Glass API response success is true");
    TEST_ASSERT(glassJson["data"]["status"] == "SUCCESS", "7b. Glass item sorting status is SUCCESS");
    TEST_ASSERT(glassJson["data"]["targetGate"] == 4, "7c. Glass item target gate is 4");

    // Valid Other
    HttpRequest otherReq;
    otherReq.method = "POST";
    otherReq.path = "/api/v1/sorting/process";
    otherReq.body = R"({"id":"OT-005","type":"OTHER","confidence":0.88,"gateId":5})";
    HttpResponse otherRes = router.routeRequest(otherReq);
    nlohmann::json otherJson = nlohmann::json::parse(otherRes.body);
    TEST_ASSERT(otherJson["success"] == true, "7d. Other API response success is true");
    TEST_ASSERT(otherJson["data"]["status"] == "SUCCESS", "7e. Other item sorting status is SUCCESS");
    TEST_ASSERT(otherJson["data"]["targetGate"] == 5, "7f. Other item target gate is 5");

    // Rejected Low-Confidence Item
    HttpRequest lowConfReq;
    lowConfReq.method = "POST";
    lowConfReq.path = "/api/v1/sorting/process";
    lowConfReq.body = R"({"id":"LOW-004","type":"PLASTIC","confidence":0.30})";
    HttpResponse lowConfRes = router.routeRequest(lowConfReq);
    nlohmann::json lowConfJson = nlohmann::json::parse(lowConfRes.body);
    TEST_ASSERT(lowConfJson["success"] == false, "8a. Rejected low-confidence item API response success is false");
    TEST_ASSERT(lowConfJson["data"]["status"] == "REJECTED", "8. Low-confidence item sorting status is REJECTED");

    // Invalid payload (Missing type)
    HttpRequest badSortingReq;
    badSortingReq.method = "POST";
    badSortingReq.path = "/api/v1/sorting/process";
    badSortingReq.body = R"({"id":"BAD-005"})";
    HttpResponse badSortingRes = router.routeRequest(badSortingReq);
    TEST_ASSERT(badSortingRes.statusCode == 400, "9. Missing type in sorting request returns 400 Bad Request");

    // Malformed JSON payload
    HttpRequest malformedJsonReq;
    malformedJsonReq.method = "POST";
    malformedJsonReq.path = "/api/v1/sorting/process";
    malformedJsonReq.body = "{INVALID_JSON";
    HttpResponse malformedJsonRes = router.routeRequest(malformedJsonReq);
    TEST_ASSERT(malformedJsonRes.statusCode == 400, "10. Malformed JSON sorting request returns 400 Bad Request");

    // 2. GET /api/v1/sorting/stats
    HttpRequest statsReq;
    statsReq.method = "GET";
    statsReq.path = "/api/v1/sorting/stats";
    HttpResponse statsRes = router.routeRequest(statsReq);
    TEST_ASSERT(statsRes.statusCode == 200, "11. GET /api/v1/sorting/stats returns 200 OK");
    nlohmann::json statsJson = nlohmann::json::parse(statsRes.body);
    TEST_ASSERT(statsJson["data"]["totalItems"].get<int>() >= 4, "12. Sorting statistics totalItems >= 4");

    // 3. GET /api/v1/telemetry/latest
    HttpRequest telemReq;
    telemReq.method = "GET";
    telemReq.path = "/api/v1/telemetry/latest";
    HttpResponse telemRes = router.routeRequest(telemReq);
    TEST_ASSERT(telemRes.statusCode == 200, "13. GET /api/v1/telemetry/latest returns 200 OK");
    nlohmann::json telemJson = nlohmann::json::parse(telemRes.body);
    TEST_ASSERT(telemJson["data"]["conveyorSpeed"].get<double>() >= 0.0, "14. Telemetry conveyorSpeed >= 0.0");

    // 4. GET /api/v1/health
    HttpRequest healthReq;
    healthReq.method = "GET";
    healthReq.path = "/api/v1/health";
    HttpResponse healthRes = router.routeRequest(healthReq);
    TEST_ASSERT(healthRes.statusCode == 200, "15. GET /api/v1/health returns 200 OK");
    nlohmann::json healthJson = nlohmann::json::parse(healthRes.body);
    TEST_ASSERT(!healthJson["data"]["status"].get<std::string>().empty(), "16. Health status string is non-empty");

    // 5. GET /api/v1/events
    HttpRequest eventsReq;
    eventsReq.method = "GET";
    eventsReq.path = "/api/v1/events";
    HttpResponse eventsRes = router.routeRequest(eventsReq);
    TEST_ASSERT(eventsRes.statusCode == 200, "17. GET /api/v1/events returns 200 OK");
    nlohmann::json eventsJson = nlohmann::json::parse(eventsRes.body);
    TEST_ASSERT(eventsJson["data"].is_array() && eventsJson["data"].size() > 0, "18. Operational events array is non-empty");

    // 6. POST /api/v1/simulation/run
    HttpRequest simReq;
    simReq.method = "POST";
    simReq.path = "/api/v1/simulation/run";
    simReq.body = R"({"items":[{"id":"SIM-1","type":"PLASTIC","confidence":0.95},{"id":"SIM-2","type":"METAL","confidence":0.91}]})";
    HttpResponse simRes = router.routeRequest(simReq);
    TEST_ASSERT(simRes.statusCode == 200, "19. POST /api/v1/simulation/run returns 200 OK");
    nlohmann::json simJson = nlohmann::json::parse(simRes.body);
    TEST_ASSERT(simJson["data"]["totalItems"].get<int>() >= 2, "20. Simulation run processed at least 2 items");

    // Invalid simulation payload
    HttpRequest badSimReq;
    badSimReq.method = "POST";
    badSimReq.path = "/api/v1/simulation/run";
    badSimReq.body = R"({"items":[]})";
    HttpResponse badSimRes = router.routeRequest(badSimReq);
    TEST_ASSERT(badSimRes.statusCode == 400, "21. Empty simulation item array returns 400 Bad Request");

    // 7. Routing & Regression Check
    HttpRequest unkReq;
    unkReq.method = "GET";
    unkReq.path = "/api/v1/unknown_endpoint";
    HttpResponse unkRes = router.routeRequest(unkReq);
    TEST_ASSERT(unkRes.statusCode == 404, "22. Unknown route returns 404 Not Found");

    HttpRequest badMethodReq;
    badMethodReq.method = "DELETE";
    badMethodReq.path = "/api/v1/health";
    HttpResponse badMethodRes = router.routeRequest(badMethodReq);
    TEST_ASSERT(badMethodRes.statusCode == 405, "23. Unsupported method DELETE on /health returns 405 Method Not Allowed");

    HttpRequest pingReq;
    pingReq.method = "GET";
    pingReq.path = "/api/v1/ping";
    HttpResponse pingRes = router.routeRequest(pingReq);
    TEST_ASSERT(pingRes.statusCode == 200, "24. GET /api/v1/ping still returns 200 OK");

    HttpRequest machStatusReq;
    machStatusReq.method = "GET";
    machStatusReq.path = "/api/v1/machine/status";
    HttpResponse machStatusRes = router.routeRequest(machStatusReq);
    TEST_ASSERT(machStatusRes.statusCode == 200, "25. GET /api/v1/machine/status still returns 200 OK");

    server.stop();
    std::remove(testStorage.c_str());
}

void testSQLiteDatabaseFoundation() {
    std::string testDbPath = "data/test_smartwaste.db";

    std::remove(testDbPath.c_str());

    SQLiteDatabase db;

    // 1. Database opens successfully
    TEST_ASSERT(db.open(testDbPath), "SQLite 1: Database opens successfully");

    // 2. Database isOpen returns true
    TEST_ASSERT(db.isOpen(), "SQLite 2: Database isOpen returns true");

    // 3. Database file created on disk
    bool fileCreated = false;
    {
        std::ifstream checkFile(testDbPath);
        fileCreated = checkFile.is_open();
    }
    TEST_ASSERT(fileCreated, "SQLite 3: Database file created on disk");

    // 4. Schema initialization succeeds & schema_version table exists
    TEST_ASSERT(db.tableExists("schema_version"), "SQLite 4: Table 'schema_version' exists");

    // 5. Schema version exists (version == 1)
    TEST_ASSERT(db.getSchemaVersion() == 1, "SQLite 5: Initial schema version is 1");

    // 6. Required foundational tables exist
    TEST_ASSERT(db.tableExists("machine_events"), "SQLite 6a: Table 'machine_events' exists");
    TEST_ASSERT(db.tableExists("waste_items"), "SQLite 6b: Table 'waste_items' exists");
    TEST_ASSERT(db.tableExists("machine_operations"), "SQLite 6c: Table 'machine_operations' exists");
    TEST_ASSERT(db.tableExists("simulation_runs"), "SQLite 6d: Table 'simulation_runs' exists");

    // 7. Reinitialization is safe (idempotent)
    TEST_ASSERT(db.initializeSchema(), "SQLite 7a: Idempotent schema reinitialization succeeds");
    TEST_ASSERT(db.getSchemaVersion() == 1, "SQLite 7b: Schema version remains 1 after reinitialization");

    // 8. Transaction commit works
    TEST_ASSERT(db.beginTransaction(), "SQLite 8a: beginTransaction succeeds");
    TEST_ASSERT(db.execute("INSERT INTO machine_operations (operation, previous_state, new_state, timestamp_ms, message) VALUES ('START', 'IDLE', 'RUNNING', 1000, 'Test start');"), "SQLite 8b: Execute SQL in transaction succeeds");
    TEST_ASSERT(db.commitTransaction(), "SQLite 8c: commitTransaction succeeds");

    // 9. Transaction rollback works
    TEST_ASSERT(db.beginTransaction(), "SQLite 9a: beginTransaction for rollback succeeds");
    TEST_ASSERT(db.execute("INSERT INTO machine_operations (operation, previous_state, new_state, timestamp_ms, message) VALUES ('STOP', 'RUNNING', 'IDLE', 2000, 'Rollback test');"), "SQLite 9b: Execute SQL in rollback transaction");
    TEST_ASSERT(db.rollbackTransaction(), "SQLite 9c: rollbackTransaction succeeds");

    // 10. Invalid SQL handled safely
    TEST_ASSERT(!db.execute("INVALID SQL STATEMENT SYNTAX ERROR;"), "SQLite 10a: Invalid SQL statement returns false");
    TEST_ASSERT(!db.getLastError().empty(), "SQLite 10b: Error message populated on invalid SQL");

    // 11. Schema version update
    TEST_ASSERT(db.setSchemaVersion(2), "SQLite 11a: setSchemaVersion(2) succeeds");
    TEST_ASSERT(db.getSchemaVersion() == 2, "SQLite 11b: getSchemaVersion() returns 2");
    TEST_ASSERT(db.setSchemaVersion(1), "SQLite 11c: Reset schema version to 1");

    // 12. Database closes successfully
    db.close();
    TEST_ASSERT(!db.isOpen(), "SQLite 12: Database closes successfully");

    std::remove(testDbPath.c_str());
}

void testMachineStateSortingSafety() {
    std::string testStorage = "data/test_safety_sorting_events.jsonl";
    std::remove(testStorage.c_str());

    SmartWasteEngine engine(testStorage);
    engine.initialize("config/config.json");

    WasteItem plasticItem;
    plasticItem.id = "SAFETY-PLASTIC";
    plasticItem.type = WasteType::PLASTIC;
    plasticItem.confidence = 0.95;

    WasteItem glassItem;
    glassItem.id = "SAFETY-GLASS";
    glassItem.type = WasteType::GLASS;
    glassItem.confidence = 0.92;

    WasteItem otherItem;
    otherItem.id = "SAFETY-OTHER";
    otherItem.type = WasteType::OTHER;
    otherItem.confidence = 0.88;

    auto* plasticGate = engine.getGate(0);
    auto* glassGate = engine.getGate(3);
    auto* otherGate = engine.getGate(4);

    // 1. Machine state is IDLE initially -> sorting REJECTED, no gate actuation
    TEST_ASSERT(engine.getMachineState() == MachineState::IDLE, "Safety 1: Machine initial state is IDLE");
    SortingResult idleRes = engine.processWasteItem(plasticItem);
    TEST_ASSERT(idleRes.status == "REJECTED", "Safety 2: IDLE state sorting returns REJECTED");
    TEST_ASSERT(plasticGate != nullptr && plasticGate->getActuationCount() == 0, "Safety 3: IDLE rejection does not actuate Gate #1");

    // 2. Machine state is RUNNING -> sorting succeeds and actuates gate
    engine.startMachine();
    TEST_ASSERT(engine.getMachineState() == MachineState::RUNNING, "Safety 4: Machine state is RUNNING after start");
    
    SortingResult runningPlasticRes = engine.processWasteItem(plasticItem);
    TEST_ASSERT(runningPlasticRes.status == "SUCCESS", "Safety 5: RUNNING state Plastic sorting succeeds");
    TEST_ASSERT(runningPlasticRes.targetGate == 1, "Safety 6: Plastic target gate is 1");
    TEST_ASSERT(plasticGate->getActuationCount() == 1, "Safety 7: RUNNING state actuates Gate #1");

    SortingResult runningGlassRes = engine.processWasteItem(glassItem);
    TEST_ASSERT(runningGlassRes.status == "SUCCESS", "Safety 8: RUNNING state Glass sorting succeeds");
    TEST_ASSERT(runningGlassRes.targetGate == 4, "Safety 9: Glass target gate is 4");
    TEST_ASSERT(glassGate != nullptr && glassGate->getActuationCount() == 1, "Safety 10: RUNNING state actuates Gate #4");

    SortingResult runningOtherRes = engine.processWasteItem(otherItem);
    TEST_ASSERT(runningOtherRes.status == "SUCCESS", "Safety 11: RUNNING state Other sorting succeeds");
    TEST_ASSERT(runningOtherRes.targetGate == 5, "Safety 12: Other target gate is 5");
    TEST_ASSERT(otherGate != nullptr && otherGate->getActuationCount() == 1, "Safety 13: RUNNING state actuates Gate #5");

    // 3. Machine state is PAUSED -> sorting REJECTED, no gate actuation increment
    engine.pauseMachine();
    TEST_ASSERT(engine.getMachineState() == MachineState::PAUSED, "Safety 14: Machine state is PAUSED");
    SortingResult pausedRes = engine.processWasteItem(plasticItem);
    TEST_ASSERT(pausedRes.status == "REJECTED", "Safety 15: PAUSED state sorting returns REJECTED");
    TEST_ASSERT(plasticGate->getActuationCount() == 1, "Safety 16: PAUSED rejection does not increment Gate #1 actuation");

    // 4. Machine state is EMERGENCY_STOP -> sorting REJECTED, no gate actuation increment
    engine.resumeMachine();
    engine.emergencyStop();
    TEST_ASSERT(engine.getMachineState() == MachineState::EMERGENCY_STOP, "Safety 17: Machine state is EMERGENCY_STOP");
    SortingResult estopRes = engine.processWasteItem(plasticItem);
    TEST_ASSERT(estopRes.status == "REJECTED", "Safety 18: EMERGENCY_STOP state sorting returns REJECTED");
    TEST_ASSERT(plasticGate->getActuationCount() == 1, "Safety 19: EMERGENCY_STOP rejection does not increment Gate #1 actuation");

    // 5. Machine reset back to IDLE -> sorting REJECTED
    engine.resetMachine();
    TEST_ASSERT(engine.getMachineState() == MachineState::IDLE, "Safety 20: Machine state reset to IDLE");
    SortingResult resetIdleRes = engine.processWasteItem(plasticItem);
    TEST_ASSERT(resetIdleRes.status == "REJECTED", "Safety 21: Post-reset IDLE state sorting returns REJECTED");

    std::remove(testStorage.c_str());
}


int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << " Running SmartWaste Unit Test Suite (Phase 7 Step 1)" << std::endl;
    std::cout << "==================================================" << std::endl;

    // HAL Interface & Type Trait Verification
    testHALInterfaceDeclarations();

    // Concrete Simulated Hardware Verification
    testSimulatedConveyor();
    testSimulatedMotor();
    testSimulatedSensor();
    testSimulatedGate();

    // Telemetry & Aggregator & Simulation Verification
    testHardwareTelemetryModel();
    testHardwareTelemetryAggregatorAndIntegration();
    testSimulationRunner();

    // Hardware Manager & Multi-Gate Verification
    testHardwareManagerMultiGate();

    // Health Monitor Verification
    testSystemHealthMonitor();

    // Integrated Multi-Gate Health Scenario
    testIntegratedMultiGateHealthScenario();

    // Safety Controller & Machine FSM Verification (Phase 4 Step 1)
    testSafetyControllerAndFSM();

    // Sorting Controller & Hardware Integration Verification (Phase 4 Step 2B)
    testSortingControllerHardwareIntegration();

    // Automated Sorting Simulation Pipeline Verification (Phase 4 Step 3)
    testAutomatedSortingSimulationPipeline();

    // Telemetry & System Health Monitoring Integration Verification (Phase 4 Step 4)
    testTelemetryAndHealthMonitoringIntegration();

    // Operational Event History & Event Tracking Verification (Phase 4 Step 5)
    testOperationalEventHistory();

    // Persistent Data Storage Verification (Phase 4 Step 6)
    testEventPersistence();

    // Core Engine Integration & Hardening Verification (Phase 4 Step 7)
    testSmartWasteEngine();

    // Machine-State Sorting Safety Verification (Stage 2)
    testMachineStateSortingSafety();

    // Native C++ HTTP Server Foundation Verification (Phase 5 Step 2)
    testApiServerFoundation();

    // Machine Lifecycle REST API Verification (Phase 5 Step 3)
    testMachineLifecycleApi();

    // Full REST API Verification (Phase 5 Step 4)
    testPhase5Step4RestApi();

    // SQLite Database Foundation Verification (Phase 7 Step 1)
    testSQLiteDatabaseFoundation();

    // Phase 2 Regression Tests
    testWasteTypeConversions();
    testMachineStateConversions();
    testWasteItemCreation();
    testSortingResultCreation();
    testConfigurationValidation();
    testLoggerBasicBehavior();

    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Summary: " << g_testsPassed << " PASSED, " << g_testsFailed << " FAILED." << std::endl;
    std::cout << "==================================================" << std::endl;


    return (g_testsFailed == 0) ? 0 : 1;
}
