import React from 'react';
import { HealthMetric } from './HealthMetric';

/**
 * System health monitor component displaying overall health evaluation & component metrics.
 */
export function HealthStatusCard({ healthData, isLoading, isError, errorMessage }) {
  if (isLoading) {
    return (
      <section className="section-container" id="health-section">
        <h3 className="section-title">System Health & Fault Monitor</h3>
        <div className="alert-box checking">Evaluating system health...</div>
      </section>
    );
  }

  if (isError) {
    return (
      <section className="section-container" id="health-section">
        <h3 className="section-title">System Health & Fault Monitor</h3>
        <div className="alert-box disconnected">
          <strong>Health Monitor Error:</strong> {errorMessage || 'Health endpoint unavailable.'}
        </div>
      </section>
    );
  }

  const {
    status = 'HEALTHY',
    message = 'System hardware operating normally.',
    isGateJammed = false,
    isMotorStopped = false,
  } = healthData || {};

  let badgeClass = 'state-success';
  if (status === 'WARNING') badgeClass = 'state-warning';
  else if (status === 'CRITICAL' || status === 'FAULT') badgeClass = 'state-critical';

  return (
    <section className="section-container" id="health-section">
      <div className="section-header">
        <h3 className="section-title">System Health & Fault Monitor</h3>
        <span className={`state-badge ${badgeClass}`} id="health-status-badge">
          {status}
        </span>
      </div>

      <div className="health-card-body">
        <div className="health-evaluation-message">
          <strong>Assessment Detail:</strong> {message}
        </div>

        <div className="health-metrics-row">
          <HealthMetric
            label="Pneumatic Diverter Gates"
            isHealthy={!isGateJammed}
            warningText="GATE JAMMED"
            healthyText="CLEAR & OPERATIONAL"
            icon="🚪"
          />
          <HealthMetric
            label="Drive Motor State"
            isHealthy={true}
            isWarning={isMotorStopped}
            warningText="MOTOR STOPPED"
            healthyText="MOTOR RUNNING"
            icon="⚡"
          />
        </div>
      </div>
    </section>
  );
}
