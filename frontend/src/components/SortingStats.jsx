import React from 'react';
import { TelemetryCard } from './TelemetryCard';

/**
 * Display component showing cumulative sorting & simulation performance metrics from C++ backend.
 */
export function SortingStats({ statsData, isLoading, isError, errorMessage, onRefresh }) {
  if (isLoading) {
    return (
      <section className="section-container" id="sorting-stats-section">
        <h3 className="section-title">Sorting & Simulation Performance Statistics</h3>
        <div className="alert-box checking">Loading sorting statistics...</div>
      </section>
    );
  }

  if (isError) {
    return (
      <section className="section-container" id="sorting-stats-section">
        <h3 className="section-title">Sorting & Simulation Performance Statistics</h3>
        <div className="alert-box disconnected">
          <strong>Statistics Error:</strong> {errorMessage || 'Sorting stats endpoint unavailable.'}
        </div>
      </section>
    );
  }

  const {
    totalItems = 0,
    successfulItems = 0,
    rejectedItems = 0,
    unavailableGateItems = 0,
    failedItems = 0,
    totalGateActuations = 0,
  } = statsData || {};

  const successRate = totalItems > 0 ? Math.round((successfulItems / totalItems) * 100) : 0;

  return (
    <section className="section-container" id="sorting-stats-section">
      <div className="section-header">
        <h3 className="section-title">Sorting & Simulation Performance Statistics</h3>
        <button className="btn-industrial" onClick={onRefresh} style={{ padding: '4px 10px', fontSize: '0.8rem' }}>
          🔄 Refresh Stats
        </button>
      </div>

      <div className="metrics-grid">
        <TelemetryCard
          id="stat-total-items"
          title="Total Items Processed"
          value={totalItems}
          statusClass="normal"
          subtext="Cumulative pipeline total"
          icon="📦"
        />

        <TelemetryCard
          id="stat-successful-items"
          title="Successfully Sorted"
          value={successfulItems}
          statusClass="positive"
          subtext={`Success Rate: ${successRate}%`}
          icon="✅"
        />

        <TelemetryCard
          id="stat-rejected-items"
          title="Low Confidence Rejections"
          value={rejectedItems}
          statusClass={rejectedItems > 0 ? 'warning' : 'normal'}
          subtext="Below threshold threshold"
          icon="⚠️"
        />

        <TelemetryCard
          id="stat-unavailable-gate"
          title="Gate Unavailable"
          value={unavailableGateItems}
          statusClass={unavailableGateItems > 0 ? 'negative' : 'normal'}
          subtext="Diverter route bypassed"
          icon="⛔"
        />

        <TelemetryCard
          id="stat-gate-actuations"
          title="Total Gate Actuations"
          value={totalGateActuations}
          statusClass="normal"
          subtext="Pneumatic diverter actions"
          icon="🚪"
        />
      </div>
    </section>
  );
}
