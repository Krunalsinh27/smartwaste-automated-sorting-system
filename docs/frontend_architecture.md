# SmartWaste React Frontend Architecture

**Project**: SmartWaste — Automated Waste Sorting & Monitoring System  
**Component**: React Frontend Foundation (Phase 6 Step 1)  

---

## 1. Overview & Purpose

The SmartWaste frontend is a web-based industrial monitoring dashboard built with **React** and **Vite**. It provides real-time visualization and control over the C++ waste sorting engine, telemetry aggregators, machine FSM state, operational events, and simulation runners.

---

## 2. Technology Stack

- **Framework**: React 19
- **Build Tool / Dev Server**: Vite 8
- **Language**: JavaScript (ES6+ JSX)
- **HTTP Client**: Native Browser Fetch API
- **Styling**: Industrial Dark Design System (Vanilla CSS with design tokens)

---

## 3. Directory Structure

```
SmartWaste/
├── docs/
│   └── frontend_architecture.md    # Frontend documentation
└── frontend/                       # Dedicated React application
    ├── public/                     # Static assets & favicons
    │   ├── favicon.svg
    │   └── robots.txt
    ├── src/
    │   ├── assets/                 # Icons & images
    │   ├── components/             # Reusable UI components
    │   │   ├── ConnectionBadge.jsx # Backend connection status badge
    │   │   ├── Header.jsx          # Industrial title bar
    │   │   └── StatusCard.jsx      # Metric / status display card
    │   ├── hooks/
    │   │   └── useApiHealth.js     # Hook for API ping health checks & polling
    │   ├── pages/
    │   │   └── Home.jsx            # Foundation overview & connection shell
    │   ├── services/
    │   │   └── api/
    │   │       └── apiClient.js    # Centralized HTTP fetch API service layer
    │   ├── styles/
    │   │   ├── index.css           # Global resets & CSS design tokens
    │   │   └── industrial.css      # Industrial layout & card styling
    │   ├── utils/
    │   │   └── formatters.js       # Formatting utilities (time, latency)
    │   ├── App.jsx                 # Root application component
    │   └── main.jsx                # React entrypoint
    ├── .env                        # Local environment settings
    ├── .env.example                # Environment variable template
    ├── index.html                  # HTML entry template
    ├── package.json                # Project dependencies & scripts
    └── vite.config.js              # Vite configuration
```

---

## 4. API Configuration & Environment Setup

All backend communications are routed through a centralized API configuration in `frontend/src/services/api/apiClient.js`.

### Environment Variable
The backend REST API base URL is configured using the environment variable:
```bash
VITE_API_BASE_URL=http://localhost:8080
```
- Defined in `frontend/.env.example` and `frontend/.env`.
- Fallback default in `apiClient.js` is `http://localhost:8080`.

---

## 5. API Service Layer & Health Check Flow

The frontend uses standard `fetch` with AbortController timeout handling.

### Connection Verification Flow (`checkApiHealth`):
1. **Trigger**: Component mount or manual refresh button click.
2. **Request**: `GET http://localhost:8080/api/v1/ping`
3. **Response Handling**:
   - `200 OK`: Status set to `connected`. UI shows LED green badge and message `"SmartWaste API is running"`.
   - `Network Error / Refused`: Status set to `disconnected`. UI shows red LED badge and helpful connection troubleshooting guidance.
   - `Timeout`: Aborted after 5000ms limit.

---

## 6. How to Run Frontend & Backend

### Prerequisites
- Node.js v18+ and npm v9+
- MinGW / C++17 build environment (Windows)

### Step 1: Run the C++ Backend Server
From the project root:
```powershell
# 1. Build the C++ executable
$env:PATH="D:\MinGW\bin;" + $env:PATH
cmake --build build

# 2. Start the HTTP API Server / Demo
.\build\smartwaste_demo.exe
```

### Step 2: Start the React Frontend Dev Server
From a separate terminal window:
```bash
# Navigate to frontend directory
cd frontend

# Install dependencies (first time only)
npm install

# Start Vite dev server
npm run dev
```
The frontend will open at `http://localhost:5173`.

### Step 3: Verify Production Build
```bash
cd frontend
npm run build
npm run preview
```

---

## 7. Frontend-Backend Communication Architecture

```
┌─────────────────────────────┐           HTTP GET           ┌─────────────────────────────┐
│    React 19 Frontend        │ ───────────────────────────> │    C++ WinSock2 HttpServer  │
│   (http://localhost:5173)   │   /api/v1/ping               │   (http://localhost:8080)   │
│                             │                              │                             │
│  - ConnectionBadge (LED)    │ <─────────────────────────── │  - ApiRouter dispatch       │
│  - StatusCard (Latency)     │           JSON               │  - Returns 200 OK           │
└─────────────────────────────┘    {"success": true, ...}    └─────────────────────────────┘
```

Both local development and CORS preflight (`OPTIONS` header) are enabled out-of-the-box by the backend `ApiRouter`.
