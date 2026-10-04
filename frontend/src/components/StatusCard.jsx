import React from 'react';

/**
 * Metric card displaying a single status parameter or metric value.
 */
export function StatusCard({ title, value, subtext, icon, id }) {
  return (
    <div className="industrial-card" id={id}>
      <div className="card-header">
        <span className="card-title">{title}</span>
        {icon && <span className="card-icon">{icon}</span>}
      </div>
      <div className="card-value">{value}</div>
      {subtext && <div className="card-subtext">{subtext}</div>}
    </div>
  );
}
