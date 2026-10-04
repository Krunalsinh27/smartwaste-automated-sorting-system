cmake_minimum_required(VERSION 3.16)
project(SmartWaste VERSION 0.4.0 LANGUAGES C CXX)


# Runtime directories and configuration files
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/config")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/data")

# Copy runtime configuration into the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config/config.json"
    "${CMAKE_BINARY_DIR}/config/config.json"
    COPYONLY
)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller
    ${CMAKE_CURRENT_SOURCE_DIR}/src/persistence
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/controllers
)

# Core Library Sources
set(CORE_SOURCES
    src/core/WasteType.cpp
    src/core/MachineState.cpp
    src/core/SystemStatus.cpp
    src/core/WasteItem.cpp
    src/core/SortingResult.cpp
    src/core/OperationalEvent.cpp
    src/core/EventHistory.cpp
    src/config/ConfigManager.cpp
    src/database/sqlite3.c
    src/database/SQLiteDatabase.cpp
    src/logging/Logger.cpp
    src/hardware/SimulatedConveyor.cpp
    src/hardware/SimulatedMotor.cpp
    src/hardware/SimulatedSensor.cpp
    src/hardware/SimulatedGate.cpp
    src/hardware/HardwareTelemetry.cpp
    src/hardware/HardwareTelemetryAggregator.cpp
    src/hardware/SimulationRunner.cpp
    src/hardware/HardwareManager.cpp
    src/hardware/SystemHealthMonitor.cpp
    src/controller/SafetyController.cpp
    src/controller/MachineFSM.cpp
    src/controller/SortingController.cpp
    src/persistence/FileEventRepository.cpp
    src/engine/SmartWasteEngine.cpp
    src/api/ApiRouter.cpp
    src/api/HttpServer.cpp
    src/api/controllers/MachineController.cpp
    src/api/controllers/SortingApiController.cpp
    src/api/controllers/TelemetryController.cpp
    src/api/controllers/HealthController.cpp
    src/api/controllers/EventController.cpp
    src/api/controllers/SimulationController.cpp
)

# Core Library Headers
set(CORE_HEADERS
    src/core/WasteType.hpp
    src/core/MachineState.hpp
    src/core/SystemStatus.hpp
    src/core/WasteItem.hpp
    src/core/SortingResult.hpp
    src/core/OperationalEvent.hpp
    src/core/EventHistory.hpp
    src/config/ConfigManager.hpp
    src/config/json.hpp
    src/database/IDatabase.hpp
    src/database/SQLiteDatabase.hpp
    src/database/sqlite3.h
    src/logging/Logger.hpp
    src/hal/IConveyor.hpp
    src/hal/IMotor.hpp
    src/hal/ISensor.hpp
    src/hal/IGate.hpp
    src/hardware/HardwareLock.hpp
    src/hardware/SimulatedConveyor.hpp
    src/hardware/SimulatedMotor.hpp
    src/hardware/SimulatedSensor.hpp
    src/hardware/SimulatedGate.hpp
    src/hardware/HardwareTelemetry.hpp
    src/hardware/HardwareTelemetryAggregator.hpp
    src/hardware/SimulationRunner.hpp
    src/hardware/HardwareManager.hpp
    src/hardware/SystemHealthMonitor.hpp
    src/controller/SafetyController.hpp
    src/controller/MachineFSM.hpp
    src/controller/SortingController.hpp
    src/persistence/IEventRepository.hpp
    src/persistence/FileEventRepository.hpp
    src/engine/SmartWasteEngine.hpp
    src/api/ApiError.hpp
    src/api/ApiRouter.hpp
    src/api/HttpServer.hpp
    src/api/controllers/MachineController.hpp
    src/api/controllers/SortingApiController.hpp
    src/api/controllers/TelemetryController.hpp
    src/api/controllers/HealthController.hpp
    src/api/controllers/EventController.hpp
    src/api/controllers/SimulationController.hpp
)


# Static Library Target for Core Subsystems
add_library(smartwaste_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Executable Target: SmartWaste Demo Console Application
add_executable(smartwaste_demo src/main.cpp)
target_link_libraries(smartwaste_demo PRIVATE smartwaste_core)

# Executable Target: SmartWaste Dedicated REST API Server
add_executable(smartwaste_server src/server_main.cpp)
target_link_libraries(smartwaste_server PRIVATE smartwaste_core)

# Executable Target: Unit Test Suite
add_executable(smartwaste_tests tests/unit_tests.cpp)
target_link_libraries(smartwaste_tests PRIVATE smartwaste_core)

# Windows Sockets Linking
if(WIN32)
    target_link_libraries(smartwaste_core PRIVATE ws2_32)
    target_link_libraries(smartwaste_demo PRIVATE ws2_32)
    target_link_libraries(smartwaste_server PRIVATE ws2_32)
    target_link_libraries(smartwaste_tests PRIVATE ws2_32)
endif()

enable_testing()

add_test(
    NAME SmartWasteUnitTests
    COMMAND smartwaste_tests
)

# Compiler Warnings
if(MSVC)
    target_compile_options(smartwaste_core PRIVATE /W4)
    target_compile_options(smartwaste_demo PRIVATE /W4)
    target_compile_options(smartwaste_server PRIVATE /W4)
    target_compile_options(smartwaste_tests PRIVATE /W4)
else()
    target_compile_options(smartwaste_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(smartwaste_demo PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(smartwaste_server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(smartwaste_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Print Configuration Summary
message(STATUS "SmartWaste Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Core Library: smartwaste_core")
message(STATUS "  Build Server App: smartwaste_server")
message(STATUS "  Build Demo App: smartwaste_demo")
message(STATUS "  Build Unit Tests: smartwaste_tests")

