import { useState, useCallback } from 'react';
import { runSimulation } from '../services/api/apiClient';

/**
 * Custom React hook for triggering and tracking batch simulation runs on the C++ SmartWaste backend.
 */
export function useSimulation() {
  const [isSimulating, setIsSimulating] = useState(false);
  const [simulationResult, setSimulationResult] = useState(null);
  const [simulationError, setSimulationError] = useState(null);
  const [lastRunTime, setLastRunTime] = useState(null);

  const executeSimulation = useCallback(async (items) => {
    if (!Array.isArray(items) || items.length === 0) {
      setSimulationError('Batch simulation payload must contain at least one waste item.');
      return { success: false, error: 'Empty item list' };
    }

    setIsSimulating(true);
    setSimulationError(null);

    const payload = { items };
    const res = await runSimulation(payload);

    setIsSimulating(false);

    if (res.success && res.data) {
      setSimulationResult(res.data);
      setSimulationError(null);
      setLastRunTime(new Date().toLocaleTimeString());
      return { success: true, data: res.data };
    } else {
      const errorMsg = res.error || 'Failed to execute batch simulation.';
      setSimulationError(errorMsg);
      return { success: false, error: errorMsg };
    }
  }, []);

  const clearResults = useCallback(() => {
    setSimulationResult(null);
    setSimulationError(null);
    setLastRunTime(null);
  }, []);

  return {
    isSimulating,
    simulationResult,
    simulationError,
    lastRunTime,
    executeSimulation,
    clearResults,
  };
}
