/**
 * SortingStatistics Component
 * Renders 5 KPI cards: TOTAL ITEMS, SUCCESSFULLY SORTED, REJECTED, GATE ACTUATIONS, SUCCESS RATE.
 * Features consistent card heights, vector icons, large metrics, and supporting text.
 */
import React from 'react';
import {
  IconBox,
  IconCheckCircle,
  IconAlertTriangle,
  IconZap,
  IconTarget,
  IconActivity
} from './Icons';

export function SortingStatistics({ statsData, isOnline, isLoading, error }) {
  if (!isOnline || error) {
    return (
      <div className="card stats-card" id="stats-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconActivity className="text-muted" size={20} />
            Performance KPIs
          </h3>
          <span className="card-badge offline">OFFLINE</span>
        </div>
        <div className="card-body">
          <div className="offline-state-message">
            <IconAlertTriangle size={24} className="text-amber" />
            <div>
              <h4>KPI DATA UNAVAILABLE</h4>
              <p>{error || 'SmartWaste backend is currently offline or unreachable.'}</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (isLoading && !statsData) {
    return (
      <div className="card stats-card" id="stats-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconActivity className="text-cyan" size={20} />
            Performance KPIs
          </h3>
          <span className="card-badge">Loading...</span>
        </div>
        <div className="card-body">
          <div className="spinner" aria-label="Loading performance KPIs"></div>
          <p className="text-center text-muted font-mono">Polling sorting statistics counters...</p>
        </div>
      </div>
    );
  }

  const {
    totalItems = 0,
    successfulItems = 0,
    rejectedItems = 0,
    totalGateActuations = 0,
  } = statsData || {};

  const successRate = totalItems > 0
    ? ((successfulItems / totalItems) * 100).toFixed(1)
    : '0.0';

  return (
    <div className="card stats-card" id="stats-card">
      <div className="card-header">
        <h3 className="card-title">
          <IconActivity className="text-cyan" size={20} />
          Performance KPIs Summary
        </h3>
        <span className="source-tag font-mono">GET /api/v1/sorting/stats</span>
      </div>

      <div className="card-body">
        <div className="kpi-grid">
          {/* 1. TOTAL ITEMS */}
          <div className="kpi-card">
            <div className="kpi-icon-header">
              <IconBox size={16} className="text-cyan" />
              <span className="kpi-label">TOTAL ITEMS</span>
            </div>
            <div className="kpi-value font-mono highlight-total">{totalItems.toLocaleString()}</div>
            <div className="kpi-sub font-mono">Total Processed</div>
          </div>

          {/* 2. SUCCESSFULLY SORTED */}
          <div className="kpi-card">
            <div className="kpi-icon-header">
              <IconCheckCircle size={16} className="text-green" />
              <span className="kpi-label">SUCCESSFUL</span>
            </div>
            <div className="kpi-value font-mono success">{successfulItems.toLocaleString()}</div>
            <div className="kpi-sub font-mono">Diverted & Sorted</div>
          </div>

          {/* 3. REJECTED */}
          <div className="kpi-card">
            <div className="kpi-icon-header">
              <IconAlertTriangle size={16} className="text-amber" />
              <span className="kpi-label">REJECTED</span>
            </div>
            <div className="kpi-value font-mono warning">{rejectedItems.toLocaleString()}</div>
            <div className="kpi-sub font-mono">&lt; 0.65 Confidence</div>
          </div>

          {/* 4. GATE ACTUATIONS */}
          <div className="kpi-card">
            <div className="kpi-icon-header">
              <IconZap size={16} className="text-cyan" />
              <span className="kpi-label">GATE ACTUATIONS</span>
            </div>
            <div className="kpi-value font-mono info">{totalGateActuations.toLocaleString()}</div>
            <div className="kpi-sub font-mono">Pneumatic Diverters</div>
          </div>

          {/* 5. SUCCESS RATE */}
          <div className="kpi-card">
            <div className="kpi-icon-header">
              <IconTarget size={16} className="text-indigo" />
              <span className="kpi-label">SUCCESS RATE</span>
            </div>
            <div className="kpi-value font-mono rate">{successRate}%</div>
            <div className="kpi-sub font-mono">{successfulItems} / {totalItems} items</div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default SortingStatistics;
