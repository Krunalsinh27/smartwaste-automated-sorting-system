# SmartWaste System — REST API Architecture Documentation

## Overview

The SmartWaste REST API provides a lightweight, native C++ HTTP/1.1 REST interface for remote monitoring and machine control. Built directly on Windows WinSock2 and POSIX socket abstractions using standard C++17, the API layer acts as a thin, thread-safe adapter pattern exposing system capabilities through the unified facade layer `smartwaste::engine::SmartWasteEngine`.

---

## Architectural Flow

```
[ HTTP Client (Web Frontend / curl / Postman) ]
                        │
                        ▼
          [ smartwaste::api::HttpServer ]
           (WinSock2 RAII, HTTP/1.1 Parser, 1MB Limit)
                        │
                        ▼
          [ smartwaste::api::ApiRouter ]
           (Method & Path Matching, CORS, Error Handlers)
                        │
       ┌────────────────┼────────────────┬────────────────┬────────────────┐
       ▼                ▼                ▼                ▼                ▼
 [MachineController] [SortingApiController] [TelemetryController] [HealthController] [EventController] [SimulationController]
       │                │                │                │                │
       └────────────────┴────────────────┼────────────────┴────────────────┘
                                         ▼
                     [ smartwaste::engine::SmartWasteEngine ]
                                         │
                    ┌────────────────────┼────────────────────┐
                    ▼                    ▼                    ▼
             [MachineFSM]      [SortingController]   [EventHistory]
```

### Architectural Principles
1. **Facade Isolation:** API controllers NEVER access low-level components (`MachineFSM`, `HardwareManager`, `SortingController`, `EventHistory`, `HardwareTelemetryAggregator`, `SystemHealthMonitor`) directly. All calls are routed through `SmartWasteEngine`.
2. **Standardized Responses:** All endpoints return structured JSON with either `{"success": true, "data": ...}` or `{"success": false, "error": {"code": "...", "message": "..."}}`.
3. **CORS Enabled:** Cross-Origin Resource Sharing is enabled by default for `http://localhost:5173` (React frontend development).

---

## Implemented Endpoints Reference

### 1. General & System Foundation

#### `GET /api/v1/ping`
- **Description:** Server health check endpoint.
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "message": "SmartWaste API is running"
  }
  ```

---

### 2. Machine Lifecycle API (`MachineController`)

#### `GET /api/v1/machine/status`
- **Description:** Fetches current machine state snapshot and subsystem flags.
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "state": "RUNNING",
      "isCameraConnected": true,
      "isConveyorRunning": true,
      "isMotorNormal": true,
      "isEmergencyStopActive": false,
      "currentDetectedWaste": "PLASTIC",
      "totalProcessedItems": 42
    }
  }
  ```

#### `POST /api/v1/machine/start`
- **Description:** Triggers machine startup sequence (`IDLE` -> `STARTING` -> `RUNNING`).
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "message": "Machine started successfully",
    "data": { "state": "RUNNING" }
  }
  ```
- **Error `400 Bad Request`:** (Invalid state transition e.g. already running)
  ```json
  {
    "success": false,
    "error": {
      "code": "INVALID_STATE_TRANSITION",
      "message": "Machine transition failed: Cannot start from state RUNNING"
    }
  }
  ```

#### `POST /api/v1/machine/pause`
- **Description:** Pauses conveyor belt (`RUNNING` -> `PAUSED`).
- **Response `200 OK`:** `{"success": true, "message": "Machine paused successfully", "data": {"state": "PAUSED"}}`

#### `POST /api/v1/machine/resume`
- **Description:** Resumes conveyor operation (`PAUSED` -> `RUNNING`).
- **Response `200 OK`:** `{"success": true, "message": "Machine resumed successfully", "data": {"state": "RUNNING"}}`

#### `POST /api/v1/machine/stop`
- **Description:** Initiates controlled machine shutdown (`RUNNING` -> `STOPPING` -> `IDLE`).
- **Response `200 OK`:** `{"success": true, "message": "Machine stopped successfully", "data": {"state": "IDLE"}}`

#### `POST /api/v1/machine/emergency-stop`
- **Description:** Asserts safety interlock immediate emergency stop (`-> EMERGENCY_STOP`).
- **Response `200 OK`:** `{"success": true, "message": "Emergency stop asserted", "data": {"state": "EMERGENCY_STOP"}}`

#### `POST /api/v1/machine/reset`
- **Description:** Clears safety faults and resets machine to `IDLE`.
- **Response `200 OK`:** `{"success": true, "message": "Machine reset successfully", "data": {"state": "IDLE"}}`

---

### 3. Waste Sorting API (`SortingApiController`)

#### `POST /api/v1/sorting/process`
- **Description:** Processes a single detected waste item through the sorting & diverter gate pipeline.
- **Request Body:**
  ```json
  {
    "id": "ITEM-101",
    "timestampMs": 1790193500000,
    "type": "PLASTIC",
    "confidence": 0.95,
    "boundingBox": { "x": 10, "y": 20, "width": 100, "height": 80 },
    "gateId": 1
  }
  ```
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "type": "PLASTIC",
      "targetGate": 1,
      "confidence": 0.95,
      "status": "SUCCESS",
      "message": "Pneumatic gate actuated cleanly."
    }
  }
  ```
- **Error `400 Bad Request`:** Missing required fields or malformed JSON payload.

#### `GET /api/v1/sorting/stats`
- **Description:** Retrieves cumulative simulation statistics and sorting counts.
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "totalItems": 10,
      "successfulItems": 8,
      "rejectedItems": 1,
      "unavailableGateItems": 1,
      "failedItems": 0,
      "totalGateActuations": 8,
      "healthChecksPerformed": 10,
      "healthyChecks": 9,
      "warningChecks": 1,
      "criticalChecks": 0
    }
  }
  ```

---

### 4. Telemetry API (`TelemetryController`)

#### `GET /api/v1/telemetry/latest`
- **Description:** Fetches real-time HAL sensor & actuator telemetry snapshot.
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "timestampMs": 1790193500000,
      "conveyorSpeed": 2.0,
      "motorRPM": 1800.0,
      "motorRunning": true,
      "sensorSignal": 0.0,
      "sensorTriggered": false,
      "gateJamDetected": false
    }
  }
  ```

---

### 5. Hardware Health API (`HealthController`)

#### `GET /api/v1/health`
- **Description:** Returns hardware diagnostic health evaluation (`HEALTHY`, `WARNING`, `FAULT`).
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "status": "HEALTHY",
      "message": "System hardware operating normally.",
      "isGateJammed": false,
      "isMotorStopped": false
    }
  }
  ```

---

### 6. Operational Events API (`EventController`)

#### `GET /api/v1/events`
- **Description:** Returns recent operational event logs recorded in system memory / persistent repository.
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": [
      {
        "eventId": "EVT-000001",
        "timestampMs": 1790193500000,
        "type": "MACHINE_STARTED",
        "severity": "INFO",
        "message": "SmartWaste machine initialized cleanly.",
        "itemId": "",
        "wasteType": "UNKNOWN",
        "gateId": 0
      }
    ]
  }
  ```

---

### 7. Simulation Execution API (`SimulationController`)

#### `POST /api/v1/simulation/run`
- **Description:** Executes a batch sequence of simulated waste items through the automated sorting pipeline.
- **Request Body:**
  ```json
  {
    "items": [
      { "id": "SIM-01", "type": "PLASTIC", "confidence": 0.95 },
      { "id": "SIM-02", "type": "METAL", "confidence": 0.91 }
    ]
  }
  ```
- **Response `200 OK`:**
  ```json
  {
    "success": true,
    "data": {
      "totalItems": 2,
      "successfulItems": 2,
      "rejectedItems": 0,
      "unavailableGateItems": 0,
      "failedItems": 0,
      "totalGateActuations": 2,
      "healthChecksPerformed": 2,
      "healthyChecks": 2,
      "warningChecks": 0,
      "criticalChecks": 0
    }
  }
  ```

---

## Standard Error Codes

| HTTP Status Code | Error Code | Description |
|---|---|---|
| `400 Bad Request` | `BAD_REQUEST` | Malformed JSON request body or missing required fields |
| `400 Bad Request` | `INVALID_STATE_TRANSITION` | Machine state transition rejected by FSM lifecycle rules |
| `404 Not Found` | `NOT_FOUND` | Requested API route does not exist |
| `405 Method Not Allowed` | `METHOD_NOT_ALLOWED` | HTTP method not permitted for target route |
| `413 Payload Too Large` | `PAYLOAD_TOO_LARGE` | HTTP request payload exceeds 1 MB limit |
| `500 Internal Server Error` | `INTERNAL_SERVER_ERROR` | Engine uninitialized or internal exception |
