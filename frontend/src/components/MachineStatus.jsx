/**
 * MachineStatus Component
 * Displays C++ FSM Machine State (IDLE, RUNNING, PAUSED, STOPPING, ERROR, EMERGENCY_STOP)
 * along with Conveyor motion status, Motor operational status, and Total Processed Items.
 */
import React from 'react';
import {
  IconCpu,
  IconGauge,
  IconGear,
  IconShield,
  IconBox,
  IconAlertTriangle
} from './Icons';

export function MachineStatus({ machineData, isOnline, isLoading, error }) {
  if (!isOnline || error) {
    return (
      <div className="card machine-status-card" id="machine-status-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconCpu className="text-muted" size={20} />
            FSM Machine State
          </h3>
          <span className="card-badge offline">OFFLINE</span>
        </div>
        <div className="card-body">
          <div className="offline-state-message">
            <IconAlertTriangle size={24} className="text-amber" />
            <div>
              <h4>MACHINE DATA UNAVAILABLE</h4>
              <p>{error || 'SmartWaste C++ backend server is offline.'}</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (isLoading && !machineData) {
    return (
      <div className="card machine-status-card" id="machine-status-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconCpu className="text-cyan" size={20} />
            FSM Machine State
          </h3>
          <span className="card-badge">Syncing...</span>
        </div>
        <div className="card-body">
          <div className="spinner" aria-label="Loading machine status"></div>
          <p className="text-center text-muted font-mono">Synchronizing with C++ Machine FSM...</p>
        </div>
      </div>
    );
  }

  const {
    state = 'UNKNOWN',
    isConveyorRunning = false,
    isMotorNormal = true,
    isEmergencyStopActive = false,
    totalProcessedItems = 0,
  } = machineData || {};

  const getStateClass = (st) => {
    switch (st) {
      case 'RUNNING': return 'state-running';
      case 'PAUSED': return 'state-paused';
      case 'IDLE': return 'state-idle';
      case 'STOPPING': return 'state-stopping';
      case 'EMERGENCY_STOP':
      case 'ERROR': return 'state-error';
      default: return 'state-unknown';
    }
  };

  return (
    <div className="card machine-status-card" id="machine-status-card">
      <div className="card-header">
        <h3 className="card-title">
          <IconCpu className="text-cyan" size={20} />
          FSM Machine State Monitor
        </h3>
        <span className="source-tag font-mono">C++ Engine FSM</span>
      </div>

      <div className="card-body">
        <div className="state-main-display">
          {/* Main State Pill Banner */}
          <div className="state-pill-container">
            <div className={`state-pill ${getStateClass(state)}`} aria-live="polite">
              <span className="state-pulse-dot" aria-hidden="true"></span>
              <span className="state-text-val">{state}</span>
            </div>
            <div className="font-mono text-muted text-right">
              <span className="text-dim">Source:</span> REST /api/v1/machine/status
            </div>
          </div>

          {/* Subsystem Indicators Grid */}
          <div className="state-sub-metrics">
            <div className="mini-metric">
              <span className="metric-label">Conveyor Belt</span>
              <span className={`metric-value ${isConveyorRunning ? 'active' : 'inactive'}`}>
                <IconGauge size={16} />
                {isConveyorRunning ? 'RUNNING' : 'STOPPED'}
              </span>
            </div>

            <div className="mini-metric">
              <span className="metric-label">Drive Motor</span>
              <span className={`metric-value ${isMotorNormal ? 'active' : 'fault'}`}>
                <IconGear size={16} />
                {isMotorNormal ? 'NORMAL' : 'FAULT'}
              </span>
            </div>

            <div className="mini-metric">
              <span className="metric-label">Safety Status</span>
              <span className={`metric-value ${isEmergencyStopActive ? 'fault' : 'active'}`}>
                <IconShield size={16} />
                {isEmergencyStopActive ? 'E-STOP' : 'CLEAR'}
              </span>
            </div>

            <div className="mini-metric">
              <span className="metric-label">Processed Items</span>
              <span className="metric-value highlight font-mono">
                <IconBox size={16} />
                {totalProcessedItems}
              </span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default MachineStatus;
