/**
 * SmartWaste System Health & Fault Monitor API
 */
import { requestApi } from './apiClient';

export const healthApi = {
  /**
   * Fetch current health monitor assessment from C++ backend
   * GET /api/v1/health
   */
  getHealth: () => requestApi('/api/v1/health', { method: 'GET' }),
};

export default healthApi;
