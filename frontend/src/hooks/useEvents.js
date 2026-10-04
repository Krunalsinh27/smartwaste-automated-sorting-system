import { useState, useEffect, useCallback, useRef } from 'react';
import { getEvents } from '../services/api/apiClient';

/**
 * Custom hook to poll real operational events stream from C++ backend.
 * @param {number} pollIntervalMs - Polling frequency in ms (default: 4000ms)
 */
export function useEvents(pollIntervalMs = 4000) {
  const [events, setEvents] = useState([]);
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

  const fetchEvents = useCallback(async () => {
    const res = await getEvents();
    if (!isMountedRef.current) return;

    if (res.success && Array.isArray(res.data)) {
      setEvents(res.data);
      setIsError(false);
      setErrorMessage('');
    } else {
      setIsError(true);
      setErrorMessage(res.error || 'Unable to retrieve operational events stream.');
    }
    setIsLoading(false);
  }, []);

  useEffect(() => {
    fetchEvents();

    if (pollIntervalMs > 0) {
      const interval = setInterval(fetchEvents, pollIntervalMs);
      return () => clearInterval(interval);
    }
  }, [fetchEvents, pollIntervalMs]);

  return {
    events,
    isLoading,
    isError,
    errorMessage,
    refreshEvents: fetchEvents,
  };
}
