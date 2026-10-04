import { useState, useEffect, useCallback } from 'react';
import { checkApiHealth, getApiBaseUrl } from '../services/api/apiClient';

/**
 * Custom hook to monitor C++ Backend API health connection status.
 * @param {number} pollIntervalMs - Optional background polling interval in ms (0 to disable)
 */
export function useApiHealth(pollIntervalMs = 10000) {
  const [status, setStatus] = useState('checking'); // 'checking' | 'connected' | 'disconnected'
  const [details, setDetails] = useState({
    message: '',
    error: '',
    responseTimeMs: 0,
    timestamp: '',
    baseUrl: getApiBaseUrl(),
  });
  const [isRefreshing, setIsRefreshing] = useState(false);

  const verifyHealth = useCallback(async () => {
    setIsRefreshing(true);
    const result = await checkApiHealth();
    
    if (result.success) {
      setStatus('connected');
      setDetails({
        message: result.message,
        error: '',
        responseTimeMs: result.responseTimeMs,
        timestamp: result.timestamp,
        baseUrl: getApiBaseUrl(),
      });
    } else {
      setStatus('disconnected');
      setDetails({
        message: '',
        error: result.error,
        responseTimeMs: result.responseTimeMs,
        timestamp: result.timestamp,
        baseUrl: getApiBaseUrl(),
      });
    }
    setIsRefreshing(false);
  }, []);

  useEffect(() => {
    verifyHealth();

    if (pollIntervalMs > 0) {
      const interval = setInterval(verifyHealth, pollIntervalMs);
      return () => clearInterval(interval);
    }
  }, [verifyHealth, pollIntervalMs]);

  return {
    status,
    details,
    isRefreshing,
    refresh: verifyHealth,
  };
}
