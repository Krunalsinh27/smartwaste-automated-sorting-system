# Phase 2 Documentation — Core Foundation, Configuration & Logging

## 1. Executive Summary

Phase 2 establishes the C++17 architectural foundation for **SmartWaste**. This phase implements framework-independent domain models, a JSON runtime configuration manager, a thread-safe logger, CMake build scripts, a console demonstration app, and a unit test suite.

---

## 2. Implemented Components & Module Rationale

### Module 1: Core Domain Models (`src/core/`)
- **`WasteType`**: Strongly-typed `enum class` (`PLASTIC`, `METAL`, `PAPER`, `GLASS`, `OTHER`, `UNKNOWN`). Includes case-insensitive string parsing `stringToWasteType()` and conversion `wasteTypeToString()`. Invalid strings safely return `WasteType::UNKNOWN`.
- **`MachineState`**: `enum class` enforcing active operation states (`IDLE`, `STARTING`, `RUNNING`, `PAUSED`, `STOPPING`, `ERROR`, `EMERGENCY_STOP`). Safe conversion utilities `machineStateToString()` and `stringToMachineState()`.
- **`SystemStatus`**: Lightweight telemetry snapshot struct tracking machine state, camera connection status, conveyor motor status, E-stop state, current detected item, and total processed counter.
- **`WasteItem`**: Domain model encapsulating detected waste telemetry. Features a custom `BoundingBox` struct (`x`, `y`, `width`, `height`, `area()`), isolating domain models completely from OpenCV's `cv::Rect`.
- **`SortingResult`**: Struct capturing sorting decision outcomes (target gate ID, confidence score, execution status, and diagnostic message).

### Module 2: Configuration Manager (`src/config/`)
- **`ConfigManager`**: Loads, parses, and validates JSON configuration files (`config/config.json`).
- Uses single-header `nlohmann/json` dependency (`src/config/json.hpp`).
- Enforces runtime validation checks (e.g. positive conveyor speed, positive frame dimensions, valid confidence thresholds) and throws `ConfigException` on failure.

### Module 3: Thread-Safe Logger (`src/logging/`)
- **`Logger`**: Singleton logger supporting severity levels (`DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`).
- Dual sinks: Console (`std::cout`/`std::cerr`) and file output (`data/smartwaste.log`).
- Pimpl locking mechanism (`LockImpl`) for thread-safe cross-platform execution without OS header macro collisions (e.g., Windows `ERROR` macro).

### Module 4 & 5: Build System & Console Demo (`src/main.cpp`)
- Root `CMakeLists.txt` configured for C++17, compiling a static library `smartwaste_core`, a console application `smartwaste_demo`, and a unit test binary `smartwaste_tests`.

### Module 6: Unit Test Harness (`tests/unit_tests.cpp`)
- Lightweight assertion test suite verifying domain conversions, bounding box area calculations, JSON config loading, invalid file exception handling, and logger file sinks (33 test assertions passed).

---

## 3. Error Handling Architecture

- **`ConfigException`**: Thrown on missing configuration files, JSON syntax errors, missing fields, or out-of-bound field values.
- **Enum Fallbacks**: `stringToWasteType()` maps unmapped strings to `WasteType::UNKNOWN` to prevent parsing crashes; `stringToMachineState()` maps unrecognized state strings to `MachineState::ERROR`.
- **Logger Safety**: Destructors flush and safely close output streams (RAII).

---

## 4. Build, Run & Test Instructions

### Prerequisites
- C++17 Compiler (GCC >= 6.3 / MSVC / Clang)
- CMake (>= 3.16)

### Build Commands
```bash
# Configure build directory
cmake -B build -G "MinGW Makefiles"

# Compile all targets
cmake --build build
```

### Run Demo Application
```bash
./build/smartwaste_demo.exe
```

### Run Unit Test Suite
```bash
./build/smartwaste_tests.exe
```

---

## 5. Example Execution Log Output

```text
[2026-09-22 17:21:26] [INFO] [Main] Starting SmartWaste — Industrial Sorting Machine Demo
[2026-09-22 17:21:26] [INFO] [Config] Loading system configuration from config/config.json...
[2026-09-22 17:21:26] [INFO] [Config] Configuration loaded and validated successfully!
[2026-09-22 17:21:26] [INFO] [Config] Machine Name: SmartWaste Industrial Sorting Machine #1
[2026-09-22 17:21:26] [INFO] [Config] Max Conveyor Speed: 2.500000 m/s
[2026-09-22 17:21:26] [INFO] [Config] Camera Frame Dimensions: 1920x1080 @ 30 FPS
[2026-09-22 17:21:26] [INFO] [Demo] --- Demonstrating Domain Models ---
[2026-09-22 17:21:26] [INFO] [Domain] [SystemStatus] State: RUNNING | Camera: CONNECTED | Conveyor: RUNNING | Motor: OK | E-Stop: OFF | Current Item: PLASTIC | Total Items: 42
[2026-09-22 17:21:26] [INFO] [Domain] [WasteItem ID: ITEM-20260922-001] Type: PLASTIC | Conf: 94.00% | BBox: (120, 340, 200x180) | Gate: 1 | Status: PENDING
[2026-09-22 17:21:26] [INFO] [Domain] [SortingResult] Type: PLASTIC -> Target Gate: 1 | Conf: 94.00% | Status: SUCCESS | Msg: Pneumatic diverter gate #1 actuated cleanly.
```

---

## 6. Scope Intentionally Excluded from Phase 2

The following subsystems are scheduled for later phases:
- Hardware Abstraction Layer (HAL) & Simulators (Phase 3)
- Machine State Machine & Safety Engine (Phase 4)
- OpenCV Frame Acquisition & Vision Processing (Phase 5)
- Sorting Decision Controller (Phase 6)
- SQLite Telemetry Database (Phase 7)
- Qt 6 Operator Dashboard (Phase 8)
