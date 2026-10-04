import React from 'react';

/**
 * Individual component health indicator badge item.
 */
export function HealthMetric({ label, isHealthy, isWarning, warningText, healthyText, icon }) {
  let statusClass = 'health-ok';
  let text = healthyText || 'OPERATIONAL';

  if (!isHealthy) {
    statusClass = 'health-fault';
    text = warningText || 'FAULT';
  } else if (isWarning) {
    statusClass = 'health-warn';
    text = warningText || 'WARNING';
  }

  return (
    <div className={`health-metric-item ${statusClass}`}>
      <div className="health-metric-label">
        {icon && <span className="metric-icon">{icon}</span>}
        <span>{label}</span>
      </div>
      <div className="health-metric-status">{text}</div>
    </div>
  );
}
