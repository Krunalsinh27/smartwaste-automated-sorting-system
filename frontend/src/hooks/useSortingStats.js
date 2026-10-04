import { useState, useEffect, useCallback, useRef } from 'react';
import { getSortingStats } from '../services/api/apiClient';

const DEFAULT_STATS = {
  totalItems: 0,
  successfulItems: 0,
  rejectedItems: 0,
  unavailableGateItems: 0,
  failedItems: 0,
  totalGateActuations: 0,
  healthChecksPerformed: 0,
  healthyChecks: 0,
  warningChecks: 0,
  criticalChecks: 0,
};

/**
 * Custom hook to manage sorting & simulation statistics from C++ backend.
 * @param {number} pollIntervalMs - Polling frequency in ms (default: 4000ms)
 */
export function useSortingStats(pollIntervalMs = 4000) {
  const [statsData, setStatsData] = useState(DEFAULT_STATS);
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

  const fetchStats = useCallback(async () => {
    const res = await getSortingStats();
    if (!isMountedRef.current) return;

    if (res.success && res.data) {
      setStatsData(res.data);
      setIsError(false);
      setErrorMessage('');
    } else {
      setIsError(true);
      setErrorMessage(res.error || 'Sorting statistics unavailable');
    }
    setIsLoading(false);
  }, []);

  useEffect(() => {
    fetchStats();

    if (pollIntervalMs > 0) {
      const interval = setInterval(fetchStats, pollIntervalMs);
      return () => clearInterval(interval);
    }
  }, [fetchStats, pollIntervalMs]);

  return {
    statsData,
    isLoading,
    isError,
    errorMessage,
    refreshStats: fetchStats,
  };
}
