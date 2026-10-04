# Phase 3 Documentation — Hardware Abstraction Layer, Simulation & Telemetry

## 1. Executive Summary

Phase 3 establishes the electromechanical integration layer for **SmartWaste**.

- **Phase 3 Step 1**: Created pure virtual HAL interfaces (`IConveyor`, `IMotor`, `ISensor`, `IGate`).
- **Phase 3 Step 2**: Created thread-safe concrete hardware simulators (`SimulatedConveyor`, `SimulatedMotor`, `SimulatedSensor`, `SimulatedGate`) with fault injection.
- **Phase 3 Step 3**: Created the **Hardware Telemetry Aggregator** (`HardwareTelemetryAggregator`), snapshot telemetry model (`HardwareTelemetry`), and deterministic simulation runner (`SimulationRunner`).

---

## 2. Telemetry Architecture & Dependency Inversion

```
                         SmartWaste Controllers
                                    │
                                    v
                       HardwareTelemetryAggregator
                                    │
                                    v
                           HardwareTelemetry
                                    ^
                                    │
                        Hardware Abstraction Layer (HAL)
                                    │
       ┌────────────────────┬───────┴────────────┬────────────────────┐
       │                    │                    │                    │
       v                    v                    v                    v
   IConveyor             IMotor               ISensor               IGate
       │                    │                    │                    │
       v                    v                    v                    v
SimulatedConveyor    SimulatedMotor       SimulatedSensor      SimulatedGate
```

### Key Architectural Constraint:
The telemetry layer observes hardware strictly through HAL interface abstractions (`IConveyor`, `IMotor`, `ISensor`, `IGate`). It contains zero header or implementation dependencies on concrete classes (`SimulatedConveyor`, etc.). 

This architectural design enforces **Dependency Inversion**, allowing physical industrial hardware drivers (e.g. Modbus TCP, EtherCAT) to replace simulated hardware without modifying telemetry aggregation logic.

---

## 3. Subsystem Descriptions

### 1. `HardwareTelemetry` (`src/hardware/HardwareTelemetry.hpp` & `.cpp`)
- Snapshot value object:
  - `timestampMs`: Epoch millisecond timestamp.
  - `conveyorSpeed`: Conveyor speed in m/s (from `IConveyor::getSpeed()`).
  - `motorRPM`: Motor RPM (from `IMotor::getRPM()`).
  - `motorRunning`: Inferred from `(motorRPM > 0.0)` without altering `IMotor` interface.
  - `sensorSignal`: Signal level (from `ISensor::readSignal()`).
  - `sensorTriggered`: Signal threshold status (from `ISensor::isTriggered()`).
  - `gateJamDetected`: Mechanical jam status (from `IGate::isJamDetected()`).

### 2. `HardwareTelemetryAggregator` (`src/hardware/HardwareTelemetryAggregator.hpp` & `.cpp`)
- Read-only observer receiving HAL interface references in constructor:
  ```cpp
  HardwareTelemetryAggregator(
      const IConveyor& conveyor,
      const IMotor& motor,
      const ISensor& sensor,
      const IGate& gate
  );
  ```
- Method `read()` returns a snapshot `HardwareTelemetry` object without mutating hardware state.

### 3. `SimulationRunner` (`src/hardware/SimulationRunner.hpp` & `.cpp`)
- Deterministic simulation clock helper.
- Advances simulation ticks and elapsed time via `stepSimulation(duration)` without introducing uncontrolled background threads or real-time loop scheduling.

---

## 4. Verification Results

- **CMake Configuration**: Successful (`C++17`, `MinGW Makefiles`).
- **Compilation**: Clean build of `smartwaste_core`, `smartwaste_demo.exe`, and `smartwaste_tests.exe` with zero compiler warnings or errors (`-Wall -Wextra -Wpedantic`).
- **Unit Test Suite Results**:
  - Total Tests: **109**
  - Passed: **109**
  - Failed: **0**
- **Regressions**: **0**. Phase 2, Phase 3 Step 1, and Phase 3 Step 2 tests remain 100% passing.
