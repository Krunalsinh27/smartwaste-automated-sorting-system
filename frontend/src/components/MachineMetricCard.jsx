import React from 'react';

/**
 * Metric card displaying a single hardware metric or state parameter.
 */
export function MachineMetricCard({ title, value, isPositive, isNegative, isWarning, icon, id }) {
  let valueClass = 'metric-normal';
  if (isPositive) valueClass = 'metric-positive';
  if (isNegative) valueClass = 'metric-negative';
  if (isWarning) valueClass = 'metric-warning';

  return (
    <div className="industrial-card metric-card" id={id}>
      <div className="card-header">
        <span className="card-title">{title}</span>
        {icon && <span className="card-icon">{icon}</span>}
      </div>
      <div className={`card-value ${valueClass}`}>{value}</div>
    </div>
  );
}
