/**
 * HealthStatus Component
 * Displays system health assessment (HEALTHY, WARNING, CRITICAL) directly from backend GET /api/v1/health.
 * Renders clear subsystem diagnostics for Conveyor, Motor, Gate, and Sensors.
 */
import React from 'react';
import {
  IconHeartPulse,
  IconCheckCircle,
  IconAlertTriangle,
  IconXCircle,
  IconGauge,
  IconGear,
  IconBarrier,
  IconRadio
} from './Icons';

export function HealthStatus({ healthData, isOnline, isLoading, error }) {
  if (!isOnline || error) {
    return (
      <div className="card health-card" id="health-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconHeartPulse className="text-muted" size={20} />
            System Health Assessment
          </h3>
          <span className="card-badge offline">OFFLINE</span>
        </div>
        <div className="card-body">
          <div className="offline-state-message">
            <IconAlertTriangle size={24} className="text-amber" />
            <div>
              <h4>HEALTH MONITOR UNAVAILABLE</h4>
              <p>{error || 'SmartWaste backend is currently offline or unreachable.'}</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (isLoading && !healthData) {
    return (
      <div className="card health-card" id="health-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconHeartPulse className="text-cyan" size={20} />
            System Health Assessment
          </h3>
          <span className="card-badge">Evaluating...</span>
        </div>
        <div className="card-body">
          <div className="spinner" aria-label="Loading health status"></div>
          <p className="text-center text-muted font-mono">Evaluating hardware safety & health registers...</p>
        </div>
      </div>
    );
  }

  const {
    status = 'UNKNOWN',
    message = 'No health message returned.',
    isGateJammed = false,
    isMotorStopped = false,
  } = healthData || {};

  const isHealthy = status === 'HEALTHY';
  const isWarning = status === 'WARNING';
  const isCritical = status === 'CRITICAL' || status === 'FAULT';

  const getHealthBadgeClass = (st) => {
    if (st === 'HEALTHY') return 'health-healthy';
    if (st === 'WARNING') return 'health-warning';
    return 'health-critical';
  };

  return (
    <div className="card health-card" id="health-card">
      <div className="card-header">
        <h3 className="card-title">
          <IconHeartPulse className={isHealthy ? 'text-green' : isWarning ? 'text-amber' : 'text-red'} size={20} />
          System Health Assessment
        </h3>
        <span className={`health-status-pill ${getHealthBadgeClass(status)}`} id="health-status-pill">
          {isHealthy ? <IconCheckCircle size={14} /> : isWarning ? <IconAlertTriangle size={14} /> : <IconXCircle size={14} />}
          {isHealthy ? 'HEALTHY' : isWarning ? 'WARNING' : isCritical ? 'CRITICAL' : status}
        </span>
      </div>

      <div className="card-body">
        <div className="health-details">
          {/* Diagnostic Message */}
          <div className="health-message-box">
            {isHealthy ? (
              <IconCheckCircle className="text-green" size={20} />
            ) : isWarning ? (
              <IconAlertTriangle className="text-amber" size={20} />
            ) : (
              <IconXCircle className="text-red" size={20} />
            )}
            <span className="health-message-text">{message}</span>
          </div>

          {/* Subsystem Diagnostics Grid */}
          <div className="health-diagnostics-grid">
            {/* Conveyor */}
            <div className="diag-item">
              <IconGauge size={18} className={isMotorStopped ? 'text-amber' : 'text-green'} />
              <div className="diag-info">
                <span className="diag-name">Conveyor</span>
                <span className={`diag-state ${isMotorStopped ? 'warning' : 'healthy'}`}>
                  {isMotorStopped ? 'Stopped' : 'Healthy'}
                </span>
              </div>
            </div>

            {/* Motor */}
            <div className="diag-item">
              <IconGear size={18} className={isMotorStopped ? 'text-amber' : 'text-green'} />
              <div className="diag-info">
                <span className="diag-name">Drive Motor</span>
                <span className={`diag-state ${isMotorStopped ? 'warning' : 'healthy'}`}>
                  {isMotorStopped ? 'Standby' : 'Normal'}
                </span>
              </div>
            </div>

            {/* Gate */}
            <div className="diag-item">
              <IconBarrier size={18} className={isGateJammed ? 'text-red' : 'text-green'} />
              <div className="diag-info">
                <span className="diag-name">Pneumatic Gate</span>
                <span className={`diag-state ${isGateJammed ? 'critical' : 'healthy'}`}>
                  {isGateJammed ? 'Jammed' : 'Ready'}
                </span>
              </div>
            </div>

            {/* Sensors */}
            <div className="diag-item">
              <IconRadio size={18} className="text-green" />
              <div className="diag-info">
                <span className="diag-name">Optical Sensors</span>
                <span className="diag-state healthy">Operational</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default HealthStatus;
