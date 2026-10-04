import React from 'react';
import { ConnectionBadge } from './ConnectionBadge';

/**
 * Top bar navigation and branding header for industrial monitoring interface.
 */
export function Header({ status, isRefreshing }) {
  return (
    <header className="industrial-header">
      <div className="brand-section">
        <div className="brand-icon">SW</div>
        <div>
          <h1 className="brand-title">SmartWaste</h1>
          <div className="brand-subtitle">Automated Waste Sorting & Monitoring System</div>
        </div>
      </div>
      <ConnectionBadge status={status} isRefreshing={isRefreshing} />
    </header>
  );
}
