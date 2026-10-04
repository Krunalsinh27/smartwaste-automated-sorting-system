/**
 * SmartWaste Industrial Operator Dashboard Page
 * Structures the operator console into clean visual sections:
 * 1. TOP HEADER (Brand, host, connection status, latency, live indicator)
 * 2. OPERATIONS INTRO BANNER (Page title, FSM state quick badge, sync meta)
 * 3. MACHINE STATUS & CONTROLS (FSM state card & REST command buttons)
 * 4. PERFORMANCE KPIs (5 metric summary cards)
 * 5. WASTE SORTING CONSOLE (Interactive form & sorting result card)
 * 6. TELEMETRY & SYSTEM HEALTH (Hardware metrics & diagnostic health assessment)
 * 7. OPERATIONAL EVENT STREAM (Event stream table / responsive cards)
 * 8. FOOTER (API & Engine details)
 */
import React, { useState } from 'react';
import { useSmartWaste } from '../hooks/useSmartWaste';
import { sortingApi } from '../api/sortingApi';

import { ApiStatus } from '../components/ApiStatus';
import { MachineStatus } from '../components/MachineStatus';
import { MachineControls } from '../components/MachineControls';
import { SortingStatistics } from '../components/SortingStatistics';
import { WasteItemForm } from '../components/WasteItemForm';
import { SortingResultCard } from '../components/SortingResultCard';
import { TelemetryCard } from '../components/TelemetryCard';
import { HealthStatus } from '../components/HealthStatus';
import { EventLog } from '../components/EventLog';
import { SimulationDashboard } from '../components/SimulationDashboard';
import {
  IconLogo,
  IconRefresh,
  IconAlertTriangle,
  IconCheckCircle,
  IconXCircle,
  IconCpu,
  IconZap,
  IconActivity
} from '../components/Icons';

export function Dashboard() {
  const {
    apiStatus,
    machineState,
    telemetryState,
    healthState,
    statsState,
    eventsState,
    commandState,
    actions,
  } = useSmartWaste();

  // Sorting Process Interactive State
  const [sortingResult, setSortingResult] = useState(null);
  const [isSubmittingSorting, setIsSubmittingSorting] = useState(false);
  const [sortingError, setSortingError] = useState('');

  const handleProcessItem = async (itemPayload) => {
    setIsSubmittingSorting(true);
    setSortingError('');
    setSortingResult(null);

    const res = await sortingApi.processItem(itemPayload);
    setIsSubmittingSorting(false);

    if (res.success && res.data) {
      setSortingResult(res.data);
      // Synchronize stats, telemetry, machine status, and events
      actions.refreshStats();
      actions.refreshTelemetry();
      actions.refreshEvents();
      actions.refreshAll();
    } else {
      setSortingError(res.error || 'Failed to process waste item.');
    }
  };

  const isOnline = apiStatus.isOnline;
  const currentFsmState = machineState.data?.state || 'IDLE';
  const isEmergencyStopActive = currentFsmState === 'EMERGENCY_STOP';

  return (
    <div className="dashboard-container" id="smartwaste-dashboard">
      {/* 1. TOP APP BAR */}
      <header className="dashboard-header">
        <div className="header-brand">
          <div className="brand-icon-box">
            <IconLogo size={24} />
          </div>
          <div className="brand-text-block">
            <div className="brand-title">
              SMART<span className="brand-title-highlight">WASTE</span>
            </div>
            <div className="brand-subtitle">Automated Waste Sorting & Monitoring</div>
          </div>
        </div>

        <div className="header-right">
          <ApiStatus apiStatus={apiStatus} />

          <button
            className="btn-industrial btn-sm btn-reset"
            onClick={actions.refreshAll}
            disabled={commandState.isExecuting}
            id="btn-refresh-all"
            title="Synchronize all telemetry & state streams"
            aria-label="Sync All Data Streams"
          >
            <IconRefresh size={15} />
            <span>Sync All</span>
          </button>
        </div>
      </header>

      {/* MAIN CONTENT AREA */}
      <main className="dashboard-main">
        {/* PAGE INTRO / OVERVIEW HEADER */}
        <div className="page-intro-header">
          <div className="intro-title-group">
            <h2>
              <IconCpu className="text-cyan" size={24} />
              Operations Control Center
            </h2>
            <p>Real-time telemetry, classification pipeline, hardware controls & engine FSM monitor</p>
          </div>

          <div className="intro-quick-meta">
            <div className="quick-meta-pill font-mono">
              <span className="text-muted">FSM State:</span>
              <span className={`font-bold ${currentFsmState === 'RUNNING' ? 'text-green' : currentFsmState === 'EMERGENCY_STOP' ? 'text-red' : 'text-cyan'}`}>
                {currentFsmState}
              </span>
            </div>
            {apiStatus.lastChecked && (
              <div className="quick-meta-pill font-mono">
                <span className="text-muted">Sync:</span>
                <span>{apiStatus.lastChecked}</span>
              </div>
            )}
          </div>
        </div>

        {/* Command Feedback Banner */}
        {commandState.feedback && (
          <div className={`command-feedback-banner ${commandState.feedback.type}`} id="command-feedback" role="status">
            {commandState.feedback.type === 'success' ? (
              <IconCheckCircle size={20} className="text-green" />
            ) : (
              <IconXCircle size={20} className="text-red" />
            )}
            <span className="feedback-message">{commandState.feedback.message}</span>
            <button className="btn-close-feedback" onClick={actions.clearFeedback} aria-label="Close feedback">×</button>
          </div>
        )}

        {/* System Offline Banner */}
        {!isOnline && (
          <div className="system-offline-banner" id="system-offline-banner" role="alert">
            <IconAlertTriangle size={24} className="text-red" />
            <div className="offline-banner-content">
              <h4>C++ SERVER UNREACHABLE</h4>
              <p>Unable to connect to SmartWaste C++ server (<code>127.0.0.1:8080</code>). Ensure <code>smartwaste_server.exe</code> is running.</p>
            </div>
            <button className="btn-industrial btn-sm btn-reset" onClick={actions.refreshAll}>
              <IconRefresh size={14} /> Retry Connection
            </button>
          </div>
        )}

        {/* Emergency Stop Banner when state === EMERGENCY_STOP */}
        {isEmergencyStopActive && (
          <div className="global-estop-banner" id="global-estop-banner" role="alert">
            <IconAlertTriangle size={28} className="text-red" />
            <div className="estop-banner-info">
              <h4>EMERGENCY HARD STOP ACTIVE</h4>
              <p>Hardware safety lockdown triggered. Conveyor and motor halted. Click <strong>RESET MACHINE</strong> to restore operations.</p>
            </div>
            <button className="btn-industrial btn-sm btn-reset" onClick={actions.resetMachine}>
              <IconRefresh size={14} /> RESET MACHINE
            </button>
          </div>
        )}

        {/* 2 & 3. MACHINE STATUS & MACHINE CONTROLS */}
        <section className="dashboard-section" id="machine-overview-section">
          <div className="dashboard-grid grid-2col">
            <MachineStatus
              machineData={machineState.data}
              isOnline={isOnline}
              isLoading={machineState.isLoading}
              error={machineState.error}
            />

            <MachineControls
              currentState={currentFsmState}
              isOnline={isOnline}
              isExecuting={commandState.isExecuting}
              actions={actions}
            />
          </div>
        </section>

        {/* 4. PERFORMANCE KPIs */}
        <section className="dashboard-section" id="kpi-section">
          <SortingStatistics
            statsData={statsState.data}
            isOnline={isOnline}
            isLoading={statsState.isLoading}
            error={statsState.error}
          />
        </section>

        {/* 5. WASTE SORTING WORKSPACE */}
        <section className="section-container" id="sorting-pipeline-section">
          <div className="section-header">
            <h3 className="section-title">
              <IconZap className="text-cyan" size={20} />
              WASTE SORTING PIPELINE WORKSPACE
            </h3>
            <span className="live-indicator font-mono">POST /api/v1/sorting/process</span>
          </div>

          <div className="sorting-pipeline-layout">
            <WasteItemForm
              onSubmit={handleProcessItem}
              isSubmitting={isSubmittingSorting}
              isConnected={isOnline}
            />

            <SortingResultCard
              result={sortingResult}
              isSubmitting={isSubmittingSorting}
              error={sortingError}
            />
          </div>
        </section>

        {/* 6 & 7. REAL-TIME TELEMETRY & SYSTEM HEALTH */}
        <section className="dashboard-section" id="telemetry-health-section">
          <div className="dashboard-grid grid-2col">
            <TelemetryCard
              telemetryData={telemetryState.data}
              isOnline={isOnline}
              isLoading={telemetryState.isLoading}
              error={telemetryState.error}
            />

            <HealthStatus
              healthData={healthState.data}
              isOnline={isOnline}
              isLoading={healthState.isLoading}
              error={healthState.error}
            />
          </div>
        </section>

        {/* 8. OPERATIONAL EVENT STREAM */}
        <section className="dashboard-section" id="event-log-section">
          <EventLog
            events={eventsState.data}
            isOnline={isOnline}
            isLoading={eventsState.isLoading}
            error={eventsState.error}
            onRefresh={actions.refreshEvents}
          />
        </section>

        {/* 9. AUTOMATED BATCH SIMULATION */}
        <section className="dashboard-section" id="simulation-section">
          <SimulationDashboard />
        </section>
      </main>

      {/* FOOTER */}
      <footer className="dashboard-footer">
        <div>SmartWaste Industrial Operator Console — WinSock2 REST API v1</div>
        <div>C++ Engine FSM Source of Truth | 127.0.0.1:8080</div>
      </footer>
    </div>
  );
}

export default Dashboard;
