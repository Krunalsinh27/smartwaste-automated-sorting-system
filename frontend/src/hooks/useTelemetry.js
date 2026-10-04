import { useState, useEffect, useCallback, useRef } from 'react';
import { getLatestTelemetry } from '../services/api/apiClient';

const DEFAULT_TELEMETRY = {
  timestampMs: 0,
  conveyorSpeed: 0,
  motorRPM: 0,
  motorRunning: false,
  sensorSignal: 0,
  sensorTriggered: false,
  gateJamDetected: false,
};

/**
 * Custom hook to poll real-time hardware telemetry snapshot from C++ backend.
 * @param {number} pollIntervalMs - Polling frequency in ms (default: 2000ms)
 */
export function useTelemetry(pollIntervalMs = 2000) {
  const [telemetryData, setTelemetryData] = useState(DEFAULT_TELEMETRY);
  const [isLoading, setIsLoading] = useState(true);
  const [isError, setIsError] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');

  const isMountedRef = useRef(true);

  useEffect(() => {
    isMountedRef.current = true;
    return () => {
      isMountedRef.current = false;
    };
  }, []);

  const fetchTelemetry = useCallback(async () => {
    const res = await getLatestTelemetry();
    if (!isMountedRef.current) return;

    if (res.success && res.data) {
      setTelemetryData(res.data);
      setIsError(false);
      setErrorMessage('');
    } else {
      setIsError(true);
      setErrorMessage(res.error || 'Telemetry data unavailable');
    }
    setIsLoading(false);
  }, []);

  useEffect(() => {
    fetchTelemetry();

    if (pollIntervalMs > 0) {
      const interval = setInterval(fetchTelemetry, pollIntervalMs);
      return () => clearInterval(interval);
    }
  }, [fetchTelemetry, pollIntervalMs]);

  return {
    telemetryData,
    isLoading,
    isError,
    errorMessage,
    refresh: fetchTelemetry,
  };
}
