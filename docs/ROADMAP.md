# SmartWaste Development Roadmap

This document outlines the multi-phase incremental development strategy for **SmartWaste**. The project progresses from core architecture and models up to a complete industrial GUI simulation and CI/CD pipeline.

---

## 🚩 Overview of Project Phases

```
Phase 1: Architecture & Planning (CURRENT)
   ↓
Phase 2: Core Data Models, Configuration & Logging Framework
   ↓
Phase 3: Hardware Abstraction Layer (HAL) & Simulators
   ↓
Phase 4: Machine State Machine & Interlock System
   ↓
Phase 5: Vision Module & OpenCV Classifier Engine
   ↓
Phase 6: Sorting Decision Controller & Timing Engine
   ↓
Phase 7: Database Layer & SQLite Repositories
   ↓
Phase 8: Qt 6 Industrial GUI Operator Dashboard
   ↓
Phase 9: GoogleTest Unit & Integration Testing Suite
   ↓
Phase 10: CI/CD Pipeline & Deployment Packaging
   ↓
(Optional Advanced Phase): Machine Learning (YOLO/ONNX) Classification
```

---

## 📋 Phase Breakdown Details

### Phase 1: Architecture, Documentation & Environment Setup (Completed)
- [x] Define functional and non-functional requirements.
- [x] Establish directory structure and project layout.
- [x] Write core documentation (`README.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `DEVELOPMENT_GUIDELINES.md`).
- [x] Setup `.gitignore`, `LICENSE`, and GitHub folder structure.

---

### Phase 2: Core Models, Config Manager & Logging Framework
- Implement enum classes: `WasteType`, `MachineState`, `LogLevel`.
- Create data primitives: `WasteItem`, `SortingResult`, `SystemStatus`.
- Build JSON configuration parser (`ConfigManager`) using header-only JSON.
- Implement thread-safe Logger module (`ConsoleSink`, `FileSink`).
- Write initial standalone test main to verify logging and config parsing.

---

### Phase 3: Hardware Abstraction Layer (HAL) & Simulators
- Define C++ pure virtual interfaces: `IConveyor`, `IMotor`, `ISensor`, `IGate`.
- Build simulated hardware implementations: `SimulatedConveyor`, `SimulatedMotor`, `SimulatedSensor`, `SimulatedGate`.
- Implement simulated hardware telemetry, delay loops, and simulated fault injection.

---

### Phase 4: Machine State Machine & Safety System
- Implement `MachineStateMachine` validating state transitions (`IDLE`, `STARTING`, `RUNNING`, `PAUSED`, `STOPPING`, `ERROR`, `EMERGENCY_STOP`).
- Build `SafetySystem` for interlock checks and hardware fault state coercion.
- Implement `MachineController` to handle user commands (`start()`, `stop()`, `pause()`, `emergencyStop()`).

---

### Phase 5: Vision Module & OpenCV Classifier Pipeline
- Define `ICamera` interface and implement `SimulatedCamera` (loading static frame images or synthetic waste shapes).
- Implement `ImageProcessor` (grayscale, Gaussian blur, thresholding, contour extraction).
- Implement deterministic `WasteClassifier` using color space analysis (HSV), geometry (aspect ratio, circularity), and size criteria.
- Output populated `WasteItem` structures with bounding boxes and classification confidence.

---

### Phase 6: Sorting Controller & Decision Engine
- Implement `SortingDecisionEngine` to map `WasteType` to physical target gate IDs.
- Build queue-based `SortingController` that calculates actuation delay based on conveyor speed and distance.
- Trigger `IGate::actuate()` when items reach target gate position.

---

### Phase 7: SQLite Database Layer & Persistence Repositories
- Implement `DatabaseManager` using SQLite C API / modern C++ wrapper.
- Create tables: `waste_records`, `machine_events`, `error_logs`, `system_statistics`.
- Implement repository classes (`WasteRecordRepository`, `EventLogRepository`, `StatisticsRepository`).
- Ensure thread-safe asynchronous write operations.

---

### Phase 8: Qt 6 Industrial Operator Dashboard (GUI)
- Create Qt 6 main window layout with industrial dark theme styling.
- Build sub-components:
  - **Camera Feed View**: Live frame widget with OpenCV bounding box overlays.
  - **Machine Control Panel**: Start, Stop, Pause, Reset, E-Stop buttons.
  - **State Indicator Panel**: Active FSM state badge and status telemetry.
  - **Counter & Analytics Widgets**: Live counts for Plastic, Metal, Paper, Glass, Other.
  - **Event & Error Log Viewer**: Table/list views for database events and active log output.
- Wire core engine events to Qt GUI using Signals & Slots.

---

### Phase 9: GoogleTest Unit & Integration Testing Suite
- Setup CMake integration for GoogleTest framework.
- Write unit tests for:
  - Core models and Config parsing.
  - State machine transition rules.
  - Sorting decision engine logic.
  - Database CRUD repository operations.
- Write mock tests for hardware simulation (`GoogleMock`).

---

### Phase 10: CI/CD Pipeline & Linux Build Packaging
- Create GitHub Actions workflow (`.github/workflows/ci.yml`).
- Configure build, linting (`clang-format`), and test execution on Ubuntu Linux runner.
- Document deployment, CMake release configuration, and executable packaging.

---

### Optional Advanced Phase: Machine Learning Classification Upgrade
- Extend `WasteClassifier` to support ONNX Runtime C++ API or OpenCV DNN module.
- Load pre-trained lightweight object detection model (e.g., YOLOv8-nano) for complex waste item classification.
