/**
 * MachineControls Component
 * Provides hardware command buttons: START, PAUSE, RESUME, STOP, RESET, EMERGENCY STOP.
 * Features state-driven button disabling, persistent Emergency Stop active banner, and safety confirmation modal.
 */
import React, { useState } from 'react';
import {
  IconPlay,
  IconPause,
  IconResume,
  IconStop,
  IconReset,
  IconAlertTriangle,
  IconGear
} from './Icons';

export function MachineControls({ currentState, isOnline, isExecuting, actions }) {
  const [showEstopModal, setShowEstopModal] = useState(false);

  const handleStart = () => actions.startMachine();
  const handlePause = () => actions.pauseMachine();
  const handleResume = () => actions.resumeMachine();
  const handleStop = () => actions.stopMachine();
  const handleReset = () => actions.resetMachine();

  const handleConfirmEstop = () => {
    setShowEstopModal(false);
    actions.triggerEmergencyStop();
  };

  const isIdle = currentState === 'IDLE';
  const isRunning = currentState === 'RUNNING';
  const isPaused = currentState === 'PAUSED';
  const isEmergencyStop = currentState === 'EMERGENCY_STOP';
  const isError = currentState === 'ERROR';

  return (
    <div className="card machine-controls-card" id="machine-controls-card">
      <div className="card-header">
        <h3 className="card-title">
          <IconGear className="text-cyan" size={20} />
          Machine Operation Controls
        </h3>
        <span className="live-indicator font-mono">REST Commands</span>
      </div>

      <div className="card-body">
        {/* Persistent visual indication when EMERGENCY_STOP is active */}
        {isEmergencyStop && (
          <div className="estop-active-banner" id="estop-active-banner" role="alert">
            <IconAlertTriangle size={18} />
            <span>EMERGENCY STOP ACTIVE — Hardware is locked down. Click RESET to restore system.</span>
          </div>
        )}

        <div className="controls-grid">
          {/* START */}
          <button
            id="btn-machine-start"
            className="btn-industrial btn-start"
            onClick={handleStart}
            disabled={!isOnline || isExecuting || (!isIdle && !isPaused)}
            title={!isOnline ? "Backend offline" : (!isIdle && !isPaused) ? "Machine must be IDLE or PAUSED to START" : "Start conveyor belt & drive motor"}
            aria-label="Start Machine"
          >
            <IconPlay size={16} />
            <span>START</span>
          </button>

          {/* PAUSE */}
          <button
            id="btn-machine-pause"
            className="btn-industrial btn-pause"
            onClick={handlePause}
            disabled={!isOnline || isExecuting || !isRunning}
            title={!isRunning ? "Machine must be RUNNING to PAUSE" : "Pause conveyor motion"}
            aria-label="Pause Machine"
          >
            <IconPause size={16} />
            <span>PAUSE</span>
          </button>

          {/* RESUME */}
          <button
            id="btn-machine-resume"
            className="btn-industrial btn-resume"
            onClick={handleResume}
            disabled={!isOnline || isExecuting || !isPaused}
            title={!isPaused ? "Machine must be PAUSED to RESUME" : "Resume conveyor motion"}
            aria-label="Resume Machine"
          >
            <IconResume size={16} />
            <span>RESUME</span>
          </button>

          {/* STOP */}
          <button
            id="btn-machine-stop"
            className="btn-industrial btn-stop"
            onClick={handleStop}
            disabled={!isOnline || isExecuting || (isIdle && !isEmergencyStop && !isError)}
            title={isIdle ? "Machine is already IDLE" : "Stop machine operations gracefully"}
            aria-label="Stop Machine"
          >
            <IconStop size={16} />
            <span>STOP</span>
          </button>

          {/* RESET */}
          <button
            id="btn-machine-reset"
            className="btn-industrial btn-reset"
            onClick={handleReset}
            disabled={!isOnline || isExecuting || (!isEmergencyStop && !isError)}
            title={(!isEmergencyStop && !isError) ? "RESET is only required when in EMERGENCY_STOP or ERROR" : "Clear fault state and reset to IDLE"}
            aria-label="Reset Machine Fault"
          >
            <IconReset size={16} />
            <span>RESET</span>
          </button>

          {/* EMERGENCY STOP */}
          <button
            id="btn-machine-estop"
            className="btn-industrial btn-estop"
            onClick={() => setShowEstopModal(true)}
            disabled={!isOnline || isExecuting}
            title="Assert immediate Emergency Hard Stop shutdown"
            aria-label="Activate Emergency Stop"
          >
            <IconAlertTriangle size={18} />
            <span>EMERGENCY STOP</span>
          </button>
        </div>
      </div>

      {/* Safety Confirmation Modal for Emergency Stop */}
      {showEstopModal && (
        <div className="modal-overlay" id="estop-modal-overlay" role="dialog" aria-modal="true">
          <div className="modal-content estop-modal">
            <div className="modal-header">
              <IconAlertTriangle size={24} />
              <h3>CONFIRM EMERGENCY HARD STOP</h3>
            </div>
            <div className="modal-body">
              <p>Are you sure you want to activate <strong>Emergency Stop</strong>?</p>
              <p className="modal-subtext">
                This command will immediately cut drive motor power, halt conveyor motion, and assert hard safety lockdown in the C++ engine.
              </p>
            </div>
            <div className="modal-footer">
              <button
                className="btn-modal btn-cancel"
                onClick={() => setShowEstopModal(false)}
                id="btn-modal-cancel"
              >
                Cancel
              </button>
              <button
                className="btn-modal btn-confirm-danger"
                onClick={handleConfirmEstop}
                id="btn-modal-confirm-estop"
              >
                ACTIVATE EMERGENCY STOP
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default MachineControls;
