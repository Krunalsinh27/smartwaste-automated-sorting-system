import React from 'react';

/**
 * Top summary bar and controls for the operational event stream.
 */
export function EventStream({ totalLoaded, totalVisible, criticalCount, isConnected, isRefreshing, onRefresh }) {
  return (
    <div className="event-stream-header-container">
      <div className="event-stream-title-group">
        <h3 className="section-title">Operational Event Stream</h3>
        <span className={`live-indicator ${isConnected ? 'connected' : 'disconnected'}`}>
          {isConnected ? '● LIVE REST POLLING' : '● EVENT STREAM OFFLINE'}
        </span>
      </div>

      <div className="event-stream-summary-bar">
        <div className="event-summary-item">
          <span className="summary-label">Loaded Events:</span>
          <span className="summary-value">{totalLoaded}</span>
        </div>
        <div className="event-summary-item">
          <span className="summary-label">Visible Events:</span>
          <span className="summary-value">{totalVisible}</span>
        </div>
        <div className="event-summary-item">
          <span className="summary-label">Warning / Critical Alerts:</span>
          <span className={`summary-value ${criticalCount > 0 ? 'text-critical' : 'text-ok'}`}>
            {criticalCount}
          </span>
        </div>

        <button
          className="btn-industrial"
          onClick={onRefresh}
          disabled={isRefreshing}
          id="btn-refresh-events"
          style={{ padding: '6px 14px', fontSize: '0.85rem' }}
        >
          {isRefreshing ? 'Refreshing...' : '🔄 Refresh Events'}
        </button>
      </div>
    </div>
  );
}
