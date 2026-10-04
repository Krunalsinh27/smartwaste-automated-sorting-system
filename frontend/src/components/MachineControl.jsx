import React from 'react';

/**
 * Control panel component exposing real machine lifecycle triggers.
 */
export function MachineControl({ state = 'IDLE', isConnected = true, isExecuting = false, actions }) {
  const isIdle = state === 'IDLE';
  const isRunning = state === 'RUNNING';
  const isPaused = state === 'PAUSED';
  const isEmergency = state === 'EMERGENCY_STOP';
  const isFault = state === 'FAULT';

  // Button availability evaluated against MachineFSM rules
  const canStart = isConnected && !isExecuting && isIdle;
  const canPause = isConnected && !isExecuting && isRunning;
  const canResume = isConnected && !isExecuting && isPaused;
  const canStop = isConnected && !isExecuting && (isRunning || isPaused);
  const canReset = isConnected && !isExecuting && (isEmergency || isFault);
  const canEmergencyStop = isConnected && !isExecuting;

  return (
    <section className="machine-control-panel">
      <h3 className="panel-title">Machine Control Operations</h3>
      <div className="control-buttons-grid">
        <button
          className="btn-control btn-start"
          onClick={actions.start}
          disabled={!canStart}
          id="btn-machine-start"
        >
          ▶ START
        </button>

        <button
          className="btn-control btn-pause"
          onClick={actions.pause}
          disabled={!canPause}
          id="btn-machine-pause"
        >
          ⏸ PAUSE
        </button>

        <button
          className="btn-control btn-resume"
          onClick={actions.resume}
          disabled={!canResume}
          id="btn-machine-resume"
        >
          ⏯ RESUME
        </button>

        <button
          className="btn-control btn-stop"
          onClick={actions.stop}
          disabled={!canStop}
          id="btn-machine-stop"
        >
          ⏹ STOP
        </button>

        <button
          className="btn-control btn-reset"
          onClick={actions.reset}
          disabled={!canReset}
          id="btn-machine-reset"
        >
          🔄 RESET FAULT
        </button>
      </div>

      {/* Visually distinct Emergency Stop Button */}
      <div className="emergency-stop-container">
        <button
          className="btn-control btn-emergency-stop"
          onClick={actions.emergencyStop}
          disabled={!canEmergencyStop}
          id="btn-machine-emergency-stop"
        >
          ⚠️ EMERGENCY STOP ⚠️
        </button>
      </div>
    </section>
  );
}
