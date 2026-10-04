/**
 * Central Orchestration Custom Hook for SmartWaste Industrial Dashboard
 * Handles polling timers, API connection monitoring, hardware state, telemetry, health, stats, and command dispatches.
 */
import { useState, useEffect, useCallback, useRef } from 'react';
import { pingApi } from '../api/apiClient';
import { machineApi } from '../api/machineApi';
import { telemetryApi } from '../api/telemetryApi';
import { healthApi } from '../api/healthApi';
import { sortingApi } from '../api/sortingApi';
import { eventsApi } from '../api/eventsApi';

export function useSmartWaste() {
  // 1. API Connection Status State
  const [apiStatus, setApiStatus] = useState({
    isOnline: false,
    latencyMs: 0,
    message: 'Connecting to SmartWaste API...',
    lastChecked: null,
  });

  // 2. Core Subsystem States
  const [machineState, setMachineState] = useState({
    data: null,
    isLoading: true,
    error: null,
  });

  const [telemetryState, setTelemetryState] = useState({
    data: null,
    isLoading: true,
    error: null,
  });

  const [healthState, setHealthState] = useState({
    data: null,
    isLoading: true,
    error: null,
  });

  const [statsState, setStatsState] = useState({
    data: null,
    isLoading: true,
    error: null,
  });

  const [eventsState, setEventsState] = useState({
    data: [],
    isLoading: true,
    error: null,
  });

  // 3. Command Execution Feedback State
  const [commandState, setCommandState] = useState({
    isExecuting: false,
    lastCommand: null,
    feedback: null, // { type: 'success' | 'error', message: string }
  });

  const isMountedRef = useRef(true);

  // --- API Fetchers ---

  const checkPing = useCallback(async () => {
    const res = await pingApi(3000);
    if (!isMountedRef.current) return;
    setApiStatus({
      isOnline: res.isOnline,
      latencyMs: res.latencyMs,
      message: res.message,
      lastChecked: res.timestamp,
    });
  }, []);

  const fetchMachineStatus = useCallback(async () => {
    const res = await machineApi.getStatus();
    if (!isMountedRef.current) return;
    if (res.success && res.data) {
      setMachineState({ data: res.data, isLoading: false, error: null });
    } else {
      setMachineState(prev => ({
        ...prev,
        isLoading: false,
        error: res.error || 'Failed to fetch machine status',
      }));
    }
  }, []);

  const fetchTelemetry = useCallback(async () => {
    const res = await telemetryApi.getLatest();
    if (!isMountedRef.current) return;
    if (res.success && res.data) {
      setTelemetryState({ data: res.data, isLoading: false, error: null });
    } else {
      setTelemetryState(prev => ({
        ...prev,
        isLoading: false,
        error: res.error || 'Failed to fetch telemetry',
      }));
    }
  }, []);

  const fetchHealth = useCallback(async () => {
    const res = await healthApi.getHealth();
    if (!isMountedRef.current) return;
    if (res.success && res.data) {
      setHealthState({ data: res.data, isLoading: false, error: null });
    } else {
      setHealthState(prev => ({
        ...prev,
        isLoading: false,
        error: res.error || 'Failed to fetch health status',
      }));
    }
  }, []);

  const fetchStats = useCallback(async () => {
    const res = await sortingApi.getStatistics();
    if (!isMountedRef.current) return;
    if (res.success && res.data) {
      setStatsState({ data: res.data, isLoading: false, error: null });
    } else {
      setStatsState(prev => ({
        ...prev,
        isLoading: false,
        error: res.error || 'Failed to fetch sorting statistics',
      }));
    }
  }, []);

  const fetchEvents = useCallback(async () => {
    const res = await eventsApi.getEvents();
    if (!isMountedRef.current) return;
    if (res.success && Array.isArray(res.data)) {
      setEventsState({ data: res.data, isLoading: false, error: null });
    } else if (res.success && res.data?.data) {
      setEventsState({ data: res.data.data, isLoading: false, error: null });
    } else {
      setEventsState(prev => ({
        ...prev,
        isLoading: false,
        error: res.error || 'Failed to fetch operational events',
      }));
    }
  }, []);

  const refreshAll = useCallback(async () => {
    await Promise.all([
      checkPing(),
      fetchMachineStatus(),
      fetchTelemetry(),
      fetchHealth(),
      fetchStats(),
      fetchEvents(),
    ]);
  }, [checkPing, fetchMachineStatus, fetchTelemetry, fetchHealth, fetchStats, fetchEvents]);

  // --- Polling Timers Setup ---
  useEffect(() => {
    isMountedRef.current = true;

    // Initial load
    refreshAll();

    // Setup polling intervals
    const pingInterval = setInterval(checkPing, 2000);
    const machineInterval = setInterval(fetchMachineStatus, 1500);
    const telemetryInterval = setInterval(fetchTelemetry, 1500);
    const healthInterval = setInterval(fetchHealth, 2500);
    const eventsInterval = setInterval(fetchEvents, 2500);
    const statsInterval = setInterval(fetchStats, 4000);

    return () => {
      isMountedRef.current = false;
      clearInterval(pingInterval);
      clearInterval(machineInterval);
      clearInterval(telemetryInterval);
      clearInterval(healthInterval);
      clearInterval(eventsInterval);
      clearInterval(statsInterval);
    };
  }, [checkPing, fetchMachineStatus, fetchTelemetry, fetchHealth, fetchEvents, fetchStats, refreshAll]);

  // --- Command Dispatch Helper ---
  const executeCommand = useCallback(async (commandName, apiFn) => {
    setCommandState({
      isExecuting: true,
      lastCommand: commandName,
      feedback: null,
    });

    const res = await apiFn();

    if (!isMountedRef.current) return res;

    if (res.success) {
      setCommandState({
        isExecuting: false,
        lastCommand: commandName,
        feedback: {
          type: 'success',
          message: res.message || `Command '${commandName}' executed successfully.`,
        },
      });
      // Refresh machine state & telemetry immediately
      fetchMachineStatus();
      fetchTelemetry();
      fetchHealth();
      fetchEvents();
    } else {
      setCommandState({
        isExecuting: false,
        lastCommand: commandName,
        feedback: {
          type: 'error',
          message: res.error || `Failed to execute command '${commandName}'.`,
        },
      });
    }
    return res;
  }, [fetchMachineStatus, fetchTelemetry, fetchHealth, fetchEvents]);

  // Command handlers
  const startMachine = () => executeCommand('START', machineApi.start);
  const pauseMachine = () => executeCommand('PAUSE', machineApi.pause);
  const resumeMachine = () => executeCommand('RESUME', machineApi.resume);
  const stopMachine = () => executeCommand('STOP', machineApi.stop);
  const triggerEmergencyStop = () => executeCommand('EMERGENCY_STOP', machineApi.emergencyStop);
  const resetMachine = () => executeCommand('RESET', machineApi.reset);
  const clearFeedback = () => setCommandState(prev => ({ ...prev, feedback: null }));

  return {
    apiStatus,
    machineState,
    telemetryState,
    healthState,
    statsState,
    eventsState,
    commandState,
    actions: {
      startMachine,
      pauseMachine,
      resumeMachine,
      stopMachine,
      triggerEmergencyStop,
      resetMachine,
      refreshAll,
      clearFeedback,
      refreshStats: fetchStats,
      refreshEvents: fetchEvents,
      refreshTelemetry: fetchTelemetry,
    },
  };
}

export default useSmartWaste;
