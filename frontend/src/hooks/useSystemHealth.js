import { useState, useEffect, useCallback, useRef } from 'react';
import { getHealthStatus } from '../services/api/apiClient';

const DEFAULT_HEALTH = {
  status: 'UNKNOWN',
  message: 'Evaluating system health...',
  isGateJammed: false,
  isMotorStopped: false,
};

/**
 * Custom hook to poll system health evaluation snapshot from C++ backend.
 * @param {number} pollIntervalMs - Polling frequency in ms (default: 2500ms)
 */
export function useSystemHealth(pollIntervalMs = 2500) {
  const [healthData, setHealthData] = useState(DEFAULT_HEALTH);
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

  const fetchHealth = useCallback(async () => {
    const res = await getHealthStatus();
    if (!isMountedRef.current) return;

    if (res.success && res.data) {
      setHealthData(res.data);
      setIsError(false);
      setErrorMessage('');
    } else {
      setIsError(true);
      setErrorMessage(res.error || 'Health status unavailable');
    }
    setIsLoading(false);
  }, []);

  useEffect(() => {
    fetchHealth();

    if (pollIntervalMs > 0) {
      const interval = setInterval(fetchHealth, pollIntervalMs);
      return () => clearInterval(interval);
    }
  }, [fetchHealth, pollIntervalMs]);

  return {
    healthData,
    isLoading,
    isError,
    errorMessage,
    refresh: fetchHealth,
  };
}
