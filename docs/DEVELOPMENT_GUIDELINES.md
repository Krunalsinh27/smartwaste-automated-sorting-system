# SmartWaste Development & Engineering Guidelines

This document defines the C++17 coding standards, architecture principles, git conventions, and software engineering practices enforced across the **SmartWaste** project codebase.

---

## 1. C++ Language & Modern Design Standards

### Standard & Compiler Compatibility
- Target **C++17** standard (`-std=c++17`).
- Zero tolerance for compiler warnings (`-Wall -Wextra -Wpedantic` on GCC/Clang, `/W4` on MSVC).

### Memory Management & Resource Safety (RAII)
- Explicit manual `delete` or `malloc/free` are strictly prohibited.
- Use **RAII** (Resource Acquisition Is Initialization) for managing memory, file handles, mutex locks, and database connections.
- Use standard smart pointers:
  - `std::unique_ptr<T>` for single-ownership resources.
  - `std::shared_ptr<T>` only for shared ownership across module boundaries.
  - `std::weak_ptr<T>` to break reference cycles where applicable.

### Const Correctness & Value Types
- Mark methods `const` whenever they do not mutate internal object state.
- Pass complex objects by `const T&` or `std::string_view` to avoid unnecessary copies.
- Prefer `std::optional<T>` over magic null values or pointers for optional return results.
- Use `std::variant<Ts...>` or strongly typed `enum class` over raw integers or C-style unions.

### Naming Conventions
- **Classes / Structs / Enums**: `PascalCase` (e.g., `SortingDecisionEngine`, `WasteType`).
- **Methods / Functions**: `camelCase` (e.g., `processFrame()`, `calculateActuationTime()`).
- **Member Variables**: `m_` prefix followed by `camelCase` (e.g., `m_currentState`, `m_conveyorSpeed`).
- **Constant Expressions / Enums**: `ALL_CAPS` (e.g., `PLASTIC`, `EMERGENCY_STOP`).
- **Interfaces**: `I` prefix followed by `PascalCase` (e.g., `IConveyor`, `ICamera`).

---

## 2. Architectural Boundaries & Coupling Rules

```
+-------------------------------------------------------------+
| GUI Layer (Qt 6)                                            |
|   - Depends ONLY on Core Models & Controller interfaces     |
|   - NO OpenCV or SQLite header inclusions                   |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
| Controller & Business Logic Layer                           |
|   - Depends on HAL, Vision, Database pure interfaces        |
|   - NO Qt widget dependencies                               |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
| Hardware Abstraction Layer (HAL) & Core Primitives          |
|   - Zero GUI dependencies                                   |
|   - Zero raw pointer hardware access                        |
+-------------------------------------------------------------+
```

### Strict Rules:
1. **No GUI Logic in Core Engine**: Code inside `src/core/`, `src/sorting/`, `src/machine/`, `src/vision/`, or `src/hardware/` MUST NOT include `<QWidget>`, `<QPushButton>`, or any Qt UI widget headers.
2. **Interface Abstraction**: Vision detectors and sorting controllers must accept interfaces (`ICamera`, `IConveyor`) rather than concrete hardware classes.
3. **Thread Safety**: Access to shared state (e.g., `MachineState`, `WasteItem` queues) across threads must be synchronized using `std::mutex` and `std::lock_guard` / `std::unique_lock`.

---

## 3. Error Handling Policy

- Use custom exception classes derived from `std::runtime_error` for unrecoverable subsystem failures (e.g., `DatabaseConnectionException`, `HardwareInitializationException`).
- Use error return codes or `std::optional` for non-exceptional flow control (e.g., item classification confidence below threshold).
- Critical hardware safety events (E-stop trigger, gate jam) must transition the `MachineStateMachine` to `ERROR` or `EMERGENCY_STOP` state immediately.

---

## 4. Testing & Code Quality

- All business logic (state transitions, sorting math, classifier rules, config validation) must be covered by **GoogleTest** unit tests in `tests/`.
- Hardware interfaces must be testable using mock objects (`GoogleMock`).
- Automated tests must run statelessly without requiring connected camera hardware or active physical displays.

---

## 5. Git & Commit Guidelines

- **Branching Strategy**:
  - `main`: Stable, release-ready code.
  - `feature/<feature-name>`: Active development branches (e.g., `feature/vision-contour-detector`).
- **Commit Messages**:
  - Follow conventional commit style:
    - `feat: add machine state machine transition rules`
    - `fix: correct conveyer speed calculation timing offset`
    - `docs: add ARCHITECTURE.md sequence diagrams`
    - `test: add unit tests for sorting decision engine`
