# SmartWaste Architecture Specification

## 1. System Overview & Architecture Philosophy

**SmartWaste** is designed as a modular industrial software application for real-time automated waste sorting actuation, machine state management, and system telematics.

### Core Architectural Principles
- **Separation of Concerns (SoC)**: Each module fulfills a specific responsibility without leaking implementation details across layer boundaries.
- **Dependency Inversion Principle (DIP)**: High-level business logic modules depend upon abstract interfaces (e.g., `IConveyor`, `IGate`, `IEventRepository`) rather than concrete hardware drivers or specific storage mechanisms.
- **Machine State Safety Enforcement**: Waste sorting requests are permitted **ONLY** when `MachineFSM` is in `MachineState::RUNNING`. Requests in non-RUNNING states are rejected before gate actuation or hardware processing.
- **Decoupled REST Interface**: The frontend React dashboard communicates with the C++ backend strictly over a standardized HTTP REST API served by a native single-threaded WinSock2 socket server listening on port 8080.

---

## 2. System Architecture Breakdown

```
+---------------------------------------------------------------------------------+
|                           REACT 19 OPERATOR DASHBOARD                           |
|       (Machine Controls, KPI Grid, Telemetry, Health, Batch Simulation)         |
+----------------------------------------+----------------------------------------+
                                         | HTTP REST API (port 8080)
+----------------------------------------v----------------------------------------+
|                 SINGLE-THREADED NATIVE C++ WINSOCK2 HTTP SERVER                 |
|             (HttpServer: select() loop 10ms | ApiRouter: 14 Endpoints)          |
+----------------------------------------+----------------------------------------+
                                         |
+----------------------------------------v----------------------------------------+
|                               CONTROLLER LAYER                                  |
|   (MachineController, SortingApiController, TelemetryController, HealthController) |
+----------------------------------------+----------------------------------------+
                                         | Facade Call
+----------------------------------------v----------------------------------------+
|                     smartwaste::engine::SmartWasteEngine                        |
+-------------------+--------------------+--------------------+-------------------+
                    |                    |                    |
+-------------------v---+    +-----------v-----------+    +---v-------------------+
|    MACHINE FSM        |    |     SORTING LAYER     |    |   PERSISTENCE LAYER   |
| [machine] MachineFSM  |--->| [sorting] 5-Gate Engine|--->| [persistence] JSONL   |
| & SafetyController    |    | Plastic/Metal/Paper/   |    | (FileEventRepository) |
+-----------------------+    | Glass/Other (Thresh:  |    | Supporting: SQLite3   |
                             | 0.65)                 |    +-----------------------+
                             +-----------+-----------+
                                         |
                                 +-------v-------+
                                 |   HAL LAYER   |
                                 |  [hardware]   |
                                 | 5 Gate Simulators
                                 +---------------+
```

---

## 3. Module Responsibilities

### 1. Core Module (`src/core/`)
- Contains domain models, primitives, and value objects used across the engine.
- **Entities**:
  - `WasteType`: Enum class (`PLASTIC`, `METAL`, `PAPER`, `GLASS`, `OTHER`, `UNKNOWN`).
  - `MachineState`: Enum class (`IDLE`, `STARTING`, `RUNNING`, `PAUSED`, `STOPPING`, `ERROR`, `EMERGENCY_STOP`).
  - `SystemStatus`: Structure holding system health metrics, motor statuses, and active alerts.
  - `WasteItem`: Struct encapsulating item ID, confidence score, bounding box, timestamp, and assigned `WasteType`.
  - `SortingResult`: Struct detailing sorting decision, target gate index, actuation delay, and execution status (`SUCCESS`, `REJECTED`, `GATE_UNAVAILABLE`, `FAILED`).
  - `Config`: System configuration structs parsed from `config/config.json`.

### 2. REST API Layer (`src/api/`)
- Exposes system capabilities over a lightweight native C++ HTTP REST API.
- **HttpServer**: Single-threaded native C++ TCP socket server using Windows WinSock2 (`ws2_32`) and a non-blocking `select()` polling loop with a 10ms timeout.
- **ApiRouter**: Method and path routing matching HTTP requests to handler functions, CORS header injection (`Access-Control-Allow-Origin: *`), and 14 registered REST endpoints.
- **Controllers**: Thin facade adapters routing requests to `SmartWasteEngine` (`MachineController`, `SortingApiController`, `TelemetryController`, `HealthController`, `EventController`, `SimulationController`).

### 3. Sorting Module (`src/sorting/`)
- Maps classified waste items to mechanical sorting gates.
- **Sorting Gate Configuration**:
  - `Gate 1`: Plastic (`WasteType::PLASTIC`)
  - `Gate 2`: Metal (`WasteType::METAL`)
  - `Gate 3`: Paper (`WasteType::PAPER`)
  - `Gate 4`: Glass (`WasteType::GLASS`)
  - `Gate 5`: Other (`WasteType::OTHER`)
- **Confidence Threshold**: Minimum `0.65` required for gate actuation; lower confidence items are routed to default handling.

### 4. Hardware Abstraction Layer (HAL) (`src/hardware/`)
- Decouples software logic from physical hardware actuators and sensors.
- **Interfaces**: `IConveyor`, `IMotor`, `ISensor`, `IGate`.
- **Simulators**: `SimulatedConveyor`, `SimulatedMotor`, `SimulatedSensor`, `SimulatedGate` (5 software pneumatic gate instances providing simulated actuation and telemetry feedback).

### 5. Machine Subsystem & Safety (`src/machine/`)
- Implements system state machine, transition rules, and safety interlocks.
- **MachineFSM**: Validates and tracks active state across `IDLE`, `STARTING`, `RUNNING`, `PAUSED`, `STOPPING`, `ERROR`, `EMERGENCY_STOP`.
- **Safety Interlock**: Rejects all sorting requests before gate actuation unless `MachineFSM` is in `RUNNING` state.
- **SafetyController**: Evaluates sensor anomalies, motor over-current, and emergency stop triggers.

### 6. Persistence Layer (`src/persistence/` & `src/database/`)
- **FileEventRepository**: Primary live persistence store saving operational state changes and sorting events to `data/events.jsonl`.
- **SQLiteDatabase**: Supporting embedded C++ SQLite database manager used for offline unit testing and demo scripts (`src/database/sqlite3.c`).

### 7. React Operator Console (`frontend/`)
- Industrial web control dashboard built with **React 19** and **Vite 8**.
- Mounts status cards, machine controls, telemetry charts, health diagnostics, event stream logs, and an integrated **Batch Simulation Runner** (`SimulationDashboard.jsx` rendered inside `Dashboard.jsx`).

---

## 4. Machine Finite State Machine (FSM) Design

The machine operation follows a formal Finite State Machine (FSM) to ensure operational safety.

### State Definitions
| State | Description |
| :--- | :--- |
| `UNINITIALIZED` | System startup, loading configuration and initializing HAL. |
| `IDLE` | Machine powered on, hardware ready for operation. |
| `STARTING` | Pre-flight check, motor spool-up, sensor calibration. |
| `RUNNING` | Conveyor active, sorting decision loop engaged (**Sorting Allowed**). |
| `PAUSED` | Conveyor stopped, machine state preserved (**Sorting Rejected**). |
| `STOPPING` | Controlled shutdown sequence (**Sorting Rejected**). |
| `ERROR` | Fault state triggered by hardware exception or sensor timeout (**Sorting Rejected**). |
| `EMERGENCY_STOP` | Immediate power cutoff; highest safety priority (**Sorting Rejected**). |

### Valid State Transitions Matrix

| From \ To | IDLE | STARTING | RUNNING | PAUSED | STOPPING | ERROR | E-STOP |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **UNINITIALIZED**| Yes | No | No | No | No | Yes | Yes |
| **IDLE** | - | Yes | No | No | No | Yes | Yes |
| **STARTING** | No | - | Yes | No | Yes | Yes | Yes |
| **RUNNING** | No | No | - | Yes | Yes | Yes | Yes |
| **PAUSED** | No | No | Yes | - | Yes | Yes | Yes |
| **STOPPING** | Yes | No | No | No | - | Yes | Yes |
| **ERROR** | Yes | No | No | No | No | - | Yes |
| **E-STOP** | Yes | No | No | No | No | No | - |

> **Safety Rule**: Sorting is allowed **ONLY** in `RUNNING` state. `EMERGENCY_STOP` can be asserted at any time from any state. Transitioning out of `EMERGENCY_STOP` or `ERROR` requires an explicit manual operator reset command back to `IDLE`.

---

## 5. Hardware Abstraction Layer (HAL) Architecture

The HAL isolates physical electromechanical components using C++17 interfaces:

```cpp
class IGate {
public:
    virtual ~IGate() = default;
    virtual bool initialize() = 0;
    virtual bool actuate(int gateId, std::chrono::milliseconds duration) = 0;
    virtual bool isJamDetected(int gateId) const = 0;
};
```

### Benefits of HAL:
- **Simulation**: Development and automated testing execute in headless environments using `SimulatedGate` and `SimulatedConveyor`.
- **Portability**: Transitioning from simulated actuators to physical GPIO (Raspberry Pi), Industrial Modbus TCP, or EtherCAT requires only writing a concrete class implementing `IGate` without modifying `SortingController` or `SmartWasteEngine`.

---

## 6. Vision Subsystem & Physical Hardware (Planned / Future Roadmap)

> [!NOTE]
> In the current implementation, waste classification input is provided via structured JSON REST API payloads (`POST /api/v1/sorting/process` or batch simulation).
> 
> The following subsystems represent **planned future roadmap items**:
> 
> 1. **Physical Camera Capture**: USB/RTSP camera frame ingestion via OpenCV or native video capture APIs.
> 2. **Computer Vision Preprocessing**: HSV color thresholding, contour extraction, and bounding box detection.
> 3. **AI/ML Classification**: YOLO / ONNX Runtime / TensorRT visual classification models.
> 4. **Physical Actuators**: Raspberry Pi / NVIDIA Jetson GPIO pneumatic solenoid relay drivers and physical conveyor belt assembly.
