import React from 'react';

/**
 * Status indicator badge with LED dot representing backend API connection status.
 */
export function ConnectionBadge({ status, isRefreshing }) {
  let label = 'Backend Disconnected';
  let statusClass = 'disconnected';

  if (status === 'connected') {
    label = 'Backend Connected';
    statusClass = 'connected';
  } else if (status === 'checking' || isRefreshing) {
    label = 'Checking Backend...';
    statusClass = 'checking';
  }

  return (
    <div className={`connection-badge ${statusClass}`} id="backend-connection-badge">
      <span className="led-dot"></span>
      <span>{label}</span>
    </div>
  );
}
