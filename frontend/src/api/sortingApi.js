/**
 * SmartWaste Waste Sorting & Simulation Pipeline API
 */
import { requestApi } from './apiClient';

export const sortingApi = {
  /**
   * Fetch cumulative sorting performance statistics
   * GET /api/v1/sorting/stats
   */
  getStatistics: () => requestApi('/api/v1/sorting/stats', { method: 'GET' }),

  /**
   * Submit a single waste item for classification & sorting gate actuation
   * POST /api/v1/sorting/process
   */
  processItem: (itemPayload) => requestApi('/api/v1/sorting/process', {
    method: 'POST',
    body: JSON.stringify(itemPayload),
  }),

  /**
   * Run automated batch simulation scenario
   * POST /api/v1/simulation/run
   */
  runSimulation: (simulationPayload) => requestApi('/api/v1/simulation/run', {
    method: 'POST',
    body: JSON.stringify(simulationPayload),
  }),
};

export default sortingApi;
