/**
 * SmartWaste Operational Events Stream API
 */
import { requestApi } from './apiClient';

export const eventsApi = {
  /**
   * Fetch recent operational events from C++ backend
   * GET /api/v1/events
   */
  getEvents: () => requestApi('/api/v1/events', { method: 'GET' }),
};

export default eventsApi;
