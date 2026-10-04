/**
 * SmartWaste Machine Controls & FSM State API
 */
import { requestApi } from './apiClient';

export const machineApi = {
  /**
   * Fetch current FSM machine state & hardware status
   * GET /api/v1/machine/status
   */
  getStatus: () => requestApi('/api/v1/machine/status', { method: 'GET' }),

  /**
   * Spool up hardware and transition to RUNNING state
   * POST /api/v1/machine/start
   */
  start: () => requestApi('/api/v1/machine/start', { method: 'POST' }),

  /**
   * Pause conveyor belt motion
   * POST /api/v1/machine/pause
   */
  pause: () => requestApi('/api/v1/machine/pause', { method: 'POST' }),

  /**
   * Resume conveyor belt motion
   * POST /api/v1/machine/resume
   */
  resume: () => requestApi('/api/v1/machine/resume', { method: 'POST' }),

  /**
   * Graceful hardware ramp down to IDLE state
   * POST /api/v1/machine/stop
   */
  stop: () => requestApi('/api/v1/machine/stop', { method: 'POST' }),

  /**
   * Immediate emergency hard stop assertion
   * POST /api/v1/machine/emergency-stop
   */
  emergencyStop: () => requestApi('/api/v1/machine/emergency-stop', { method: 'POST' }),

  /**
   * Clear fault state and reset machine to IDLE
   * POST /api/v1/machine/reset
   */
  reset: () => requestApi('/api/v1/machine/reset', { method: 'POST' }),
};

export default machineApi;
