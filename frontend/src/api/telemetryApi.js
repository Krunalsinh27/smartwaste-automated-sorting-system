/**
 * SmartWaste Hardware Telemetry API
 */
import { requestApi } from './apiClient';

export const telemetryApi = {
  /**
   * Fetch latest real-time hardware telemetry snapshot (conveyor speed, motor RPM, sensor signal, gate jam status)
   * GET /api/v1/telemetry/latest
   */
  getLatest: () => requestApi('/api/v1/telemetry/latest', { method: 'GET' }),
};

export default telemetryApi;
