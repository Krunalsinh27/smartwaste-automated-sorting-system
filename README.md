# SmartWaste — Automated Waste Sorting & Monitoring System

![C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)
![REST API](https://img.shields.io/badge/API-WinSock2%20REST-orange.svg)
![Frontend](https://img.shields.io/badge/Frontend-React%2019%20%2B%20Vite%208-green.svg)
![Persistence](https://img.shields.io/badge/Persistence-JSONL%20%2B%20SQLite-lightgrey.svg)
![Testing](https://img.shields.io/badge/Testing-478%20Assertions%20Passed-brightgreen.svg)
![Build](https://img.shields.io/badge/Build-CMake-orange.svg)

## 📌 Project Overview

**SmartWaste** is a real-time industrial automated waste sorting & telematics simulation system built in **C++17** and **React 19**. It models an industrial waste sorting conveyor belt assembly featuring a Machine Finite State Machine (FSM), Hardware Abstraction Layer (HAL) with 5 pneumatic sorting gates, safety interlock controller, JSONL event persistence, and a single-threaded native C++ HTTP REST API server (`WinSock2` / `select()`) exposing 14 system endpoints.

An interactive **React 19 + Vite 8** operator dashboard connects to the backend, providing real-time machine FSM control, live telemetry streaming, sorting performance metrics, diagnostic health monitoring, event logging, and an integrated batch simulation runner.

Designed as a professional industrial automation software portfolio project, **SmartWaste** demonstrates modern C++ architecture, strict decoupling of GUI from core business logic, thread-safe state management, hardware abstraction for physical component simulation, and comprehensive automated test coverage.

---

## 🛠️ Technology Stack

- **Core Backend**: Modern C++17 (`std::filesystem`, `std::optional`, RAII, smart pointers)
- **REST API Server**: Single-threaded native C++ HTTP server using WinSock2 sockets and `select()` polling
- **Frontend Console**: React 19, Vite 8, JavaScript (ES6+ JSX), Vanilla CSS design tokens
- **State Machine & HAL**: Finite State Machine (`MachineFSM`), HAL interfaces (`IConveyor`, `IMotor`, `ISensor`, `IGate`), simulated actuators (5 gates: Plastic, Metal, Paper, Glass, Other)
- **Persistence**: `FileEventRepository` (`data/events.jsonl`) for live REST API event logging; `SQLiteDatabase` supporting module for offline unit tests and demo scripts
- **Build System**: CMake (>= 3.16)
- **Testing**: C++ Unit Test Suite (48 test suites, 478 passing assertions, custom header macro test runner)
- **Configuration**: JSON (`config/config.json` parsed via `nlohmann/json`)

---

## 🏗️ High-Level System Architecture

```
+-------------------------------------------------------------------+
|                    React 19 Operator Dashboard                    |
|   (Machine Controls, KPI Grid, Telemetry, Batch Simulation)       |
+---------------------------------+---------------------------------+
                                  | HTTP REST API (port 8080)
+---------------------------------v---------------------------------+
|            Single-Threaded Native C++ WinSock2 Server             |
|                  (HttpServer + ApiRouter)                         |
+---------------------------------v---------------------------------+
|                     SmartWasteEngine Facade                       |
|         (Machine FSM: IDLE, RUNNING, PAUSED, E-STOP, etc.)        |
+--------+------------------------+------------------------+--------+
         |                        |                        |
+--------v-------+       +--------v-------+       +--------v-------+
| Machine Safety |       | Sorting Module |       | Persistence    |
| Interlocks     |------>| (5-Gate Engine)|------>| (JSONL / SQLite|
+----------------+       +--------+-------+       +----------------+
                                  |
                         +--------v-------+
                         | Hardware (HAL) |
                         | (5 Gates Sim)  |
                         +----------------+
```

---

## ⚙️ System Configuration & Safety

### Sorting Gate Mapping & Rules
- **Gate 1**: Plastic (`WasteType::PLASTIC`)
- **Gate 2**: Metal (`WasteType::METAL`)
- **Gate 3**: Paper (`WasteType::PAPER`)
- **Gate 4**: Glass (`WasteType::GLASS`)
- **Gate 5**: Other (`WasteType::OTHER`)
- **Max Gates**: `5`
- **Minimum Confidence Threshold**: `0.65` (configured in `config/config.json`)

### Machine Safety Policy
Waste sorting is allowed **ONLY** when the machine is in the `RUNNING` operating state. If the machine is in any non-RUNNING state (`IDLE`, `STARTING`, `PAUSED`, `STOPPING`, `ERROR`, `EMERGENCY_STOP`), sorting requests are rejected before gate actuation or hardware execution.

---

## 🧪 Simulation vs. Real Hardware

### IMPLEMENTED SOFTWARE & SIMULATION
- Native C++ engine with 5 pneumatic gate simulation instances.
- Machine FSM controller with safety interlocks and transition verification.
- Native C++ single-threaded REST API server (14 endpoints).
- React 19 + Vite 8 industrial operator console with integrated Batch Simulation runner.
- Live event logging (`data/events.jsonl`) and SQLite supporting database foundation.
- Automated unit test suite (48 test suites, 478 passing assertions).

### SIMULATED / NOT PHYSICAL
- Waste classification input (submitted as structured JSON objects via API).
- Conveyor speed, drive motor RPM, proximity sensors, and pneumatic gate actuations.

### NOT IMPLEMENTED / FUTURE ROADMAP
- Physical camera ingestion (USB/RTSP) & OpenCV image preprocessing pipeline.
- Real-time ML/AI classification models (YOLO / ONNX Runtime / TensorRT).
- Industrial PLC / Raspberry Pi GPIO pneumatic solenoid relay drivers.
- Docker containerization, systemd service units, and cloud deployment pipelines.

---

## 📁 Directory Structure

```
SmartWaste/
│
├── CMakeLists.txt              # Root build configuration
├── README.md                   # Project overview & documentation
├── LICENSE                     # MIT License
├── .gitignore                  # Git ignore rules
│
├── src/                        # C++ Source files
│   ├── main.cpp                # Console demo entrypoint
│   ├── main_server.cpp         # REST API server entrypoint
│   ├── api/                    # WinSock2 HttpServer, ApiRouter & Controllers
│   ├── core/                   # Domain entities & data models
│   ├── sorting/                # Sorting decision engine & gate actuation logic
│   ├── hardware/               # Hardware Abstraction Layer (HAL) & simulators
│   ├── machine/                # Machine FSM & safety interlock controller
│   ├── persistence/            # FileEventRepository (JSONL) & persistence
│   ├── database/               # SQLite database manager & repositories
│   ├── logging/                # Thread-safe logging framework
│   └── config/                 # JSON configuration manager (`nlohmann/json.hpp`)
│
├── frontend/                   # React 19 + Vite 8 Operator Dashboard
│   ├── src/
│   │   ├── components/         # StatusCard, ConnectionBadge, Icons, SimulationDashboard
│   │   ├── pages/              # Dashboard.jsx
│   │   └── api/                # apiClient.js fetch wrapper
│   ├── package.json            # React 19, Vite 8, oxlint
│   └── vite.config.js          # Vite build configuration
│
├── tests/                      # Unit test suite (`unit_tests.cpp`)
├── config/                     # System JSON configuration files (`config.json`)
├── data/                       # Operational event storage (`events.jsonl`)
└── docs/                       # Architectural specifications & documentation
```

---

## 🚀 Quick Start

### 1. Build and Run C++ Backend Server
```powershell
# Configure & compile C++ binaries using CMake
cmake -B build -G "MinGW Makefiles"
cmake --build build

# Run unit test suite (478 assertions passed)
.\build\smartwaste_tests.exe

# Start the REST API server on http://localhost:8080
.\build\smartwaste_server.exe
```
*Note for Windows users*: When running `ctest` directly, ensure `D:\MinGW\bin` (or your compiler binary directory) is included in your `$env:PATH`.

### 2. Run React Operator Dashboard
```bash
cd frontend

# Install Node dependencies
npm install

# Start Vite development server at http://localhost:5173
npm run dev

# Verify production build
npm run build
```

---

## 🧪 Unit Testing Status

- **Framework**: Custom header macro test runner (`tests/unit_tests.cpp`)
- **Test Suites**: 48 test suites
- **Assertions**: **478 PASSED**, 0 FAILED
- **Execution**: Direct executable (`smartwaste_tests.exe`) and CTest integration

---

## 📜 License

Distributed under the MIT License. See [LICENSE](file:///c:/Users/kruna/OneDrive/Desktop/SmartWaste/LICENSE) for details.
