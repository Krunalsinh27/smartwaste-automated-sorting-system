import React from 'react';

/**
 * Visual badge & status card presenting current MachineFSM state.
 */
export function MachineStatusCard({ state = 'IDLE', isConnected = true }) {
  let displayState = isConnected ? state : 'DISCONNECTED';
  let badgeClass = 'state-neutral';
  let description = 'System ready for startup commands.';

  switch (displayState) {
    case 'RUNNING':
      badgeClass = 'state-success';
      description = 'Sorting conveyor & hardware running in active cycle.';
      break;
    case 'PAUSED':
      badgeClass = 'state-warning';
      description = 'Execution paused. Operations can be resumed.';
      break;
    case 'EMERGENCY_STOP':
      badgeClass = 'state-critical';
      description = 'EMERGENCY SHUTDOWN ASSERTED. System safety locked.';
      break;
    case 'FAULT':
    case 'ERROR':
      badgeClass = 'state-critical';
      description = 'Hardware fault detected. System reset required.';
      break;
    case 'DISCONNECTED':
      badgeClass = 'state-offline';
      description = 'Offline. Start SmartWaste C++ backend server.';
      break;
    case 'IDLE':
    default:
      badgeClass = 'state-neutral';
      description = 'System idle. Pre-flight checks passed.';
      break;
  }

  return (
    <div className="machine-status-card" id="machine-status-card">
      <div className="status-header">
        <span className="status-label">Machine FSM State</span>
        <span className={`state-badge ${badgeClass}`} id="machine-state-badge">
          {displayState}
        </span>
      </div>
      <div className="status-description">{description}</div>
    </div>
  );
}
