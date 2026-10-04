import { useState, useEffect, useCallback, useRef } from 'react';
import {
  getMachineStatus,
  startMachine,
  pauseMachine,
  resumeMachine,
  stopMachine,
  emergencyStop,
  resetMachine,
} from '../services/api/apiClient';

const DEFAULT_MACHINE_DATA = {
  state: 'UNKNOWN',
  isCameraConnected: false,
  isConveyorRunning: false,
  isMotorNormal: false,
  isEmergencyStopActive: false,
  currentDetectedWaste: 'UNKNOWN',
  totalProcessedItems: 0,
};

/**
 * Custom hook to manage machine status polling, state synchronization, and command execution.
 * @param {number} pollIntervalMs - Polling frequency in ms (default: 2500ms)
 */
export function useMachineStatus(pollIntervalMs = 2500) {
  const [machineData, setMachineData] = useState(DEFAULT_MACHINE_DATA);
  const [isConnected, setIsConnected] = useState(false);
  const [isLoadingStatus, setIsLoadingStatus] = useState(true);

  const [commandState, setCommandState] = useState({
    isExecuting: false,
    feedbackMessage: '',
    isError: false,
    timestamp: '',
  });

  const isMountedRef = useRef(true);

  useEffect(() => {
    isMountedRef.current = true;
    return () => {
      isMountedRef.current = false;
    };
  }, []);

  const fetchStatus = useCallback(async () => {
    const res = await getMachineStatus();
    if (!isMountedRef.current) return;

    if (res.success && res.data) {
      setIsConnected(true);
      setMachineData(res.data);
    } else {
      setIsConnected(false);
      // If network fails or connection lost
      setMachineData((prev) => ({
        ...prev,
        state: res.code === 'NETWORK_ERROR' ? 'DISCONNECTED' : prev.state,
      }));
    }
    setIsLoadingStatus(false);
  }, []);

  useEffect(() => {
    fetchStatus();

    if (pollIntervalMs > 0) {
      const interval = setInterval(fetchStatus, pollIntervalMs);
      return () => clearInterval(interval);
    }
  }, [fetchStatus, pollIntervalMs]);

  const runCommand = useCallback(
    async (commandFn, actionLabel) => {
      const timestamp = new Date().toLocaleTimeString();
      setCommandState({
        isExecuting: true,
        feedbackMessage: `Sending ${actionLabel} command to SmartWaste engine...`,
        isError: false,
        timestamp,
      });

      const result = await commandFn();
      if (!isMountedRef.current) return;

      if (result.success) {
        setCommandState({
          isExecuting: false,
          feedbackMessage: result.message || `${actionLabel} command executed successfully.`,
          isError: false,
          timestamp,
        });
      } else {
        setCommandState({
          isExecuting: false,
          feedbackMessage: result.error || `Failed to execute ${actionLabel}.`,
          isError: true,
          timestamp,
        });
      }

      // Immediately synchronize status from backend
      await fetchStatus();
    },
    [fetchStatus]
  );

  const handleStart = useCallback(() => runCommand(startMachine, 'START'), [runCommand]);
  const handlePause = useCallback(() => runCommand(pauseMachine, 'PAUSE'), [runCommand]);
  const handleResume = useCallback(() => runCommand(resumeMachine, 'RESUME'), [runCommand]);
  const handleStop = useCallback(() => runCommand(stopMachine, 'STOP'), [runCommand]);
  
  const handleEmergencyStop = useCallback(() => {
    const confirmMsg = "CRITICAL ACTION: Are you sure you want to trigger EMERGENCY STOP?\n\nThis will immediately halt the conveyor belt and drive motor.";
    if (window.confirm(confirmMsg)) {
      return runCommand(emergencyStop, 'EMERGENCY STOP');
    }
  }, [runCommand]);

  const handleReset = useCallback(() => runCommand(resetMachine, 'RESET'), [runCommand]);

  const clearFeedback = useCallback(() => {
    setCommandState((prev) => ({ ...prev, feedbackMessage: '' }));
  }, []);

  return {
    machineData,
    isConnected,
    isLoadingStatus,
    commandState,
    actions: {
      start: handleStart,
      pause: handlePause,
      resume: handleResume,
      stop: handleStop,
      emergencyStop: handleEmergencyStop,
      reset: handleReset,
      refreshStatus: fetchStatus,
      clearFeedback,
    },
  };
}
