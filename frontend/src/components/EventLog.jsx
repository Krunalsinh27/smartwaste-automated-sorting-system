/**
 * EventLog Component
 * Renders operational event log stream with severity badges (INFO, WARNING, CRITICAL).
 * On desktop: Clean scannable table.
 * On mobile/tablet: Responsive event cards preventing horizontal page overflow.
 */
import React, { useState } from 'react';
import {
  IconList,
  IconRefresh,
  IconFilter,
  IconCheckCircle,
  IconAlertTriangle,
  IconXCircle,
  IconActivity
} from './Icons';

export function EventLog({ events, isOnline, isLoading, error, onRefresh }) {
  const [filterSeverity, setFilterSeverity] = useState('ALL');

  if (!isOnline || error) {
    return (
      <div className="card event-log-card" id="event-log-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconList className="text-muted" size={20} />
            Operational Event Stream
          </h3>
          <span className="card-badge offline">OFFLINE</span>
        </div>
        <div className="card-body">
          <div className="offline-state-message">
            <IconAlertTriangle size={24} className="text-amber" />
            <div>
              <h4>EVENT LOG UNAVAILABLE</h4>
              <p>{error || 'SmartWaste backend is currently offline or unreachable.'}</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  const eventsList = Array.isArray(events) ? events : [];
  
  // Sort newest events first
  const sortedEvents = [...eventsList].reverse();

  const filteredEvents = filterSeverity === 'ALL'
    ? sortedEvents
    : sortedEvents.filter(e => e.severity === filterSeverity);

  const getSeverityBadgeClass = (sev) => {
    switch (sev) {
      case 'INFO': return 'sev-info';
      case 'WARNING': return 'sev-warning';
      case 'CRITICAL':
      case 'ERROR': return 'sev-error';
      default: return 'sev-info';
    }
  };

  const getSeverityIcon = (sev) => {
    switch (sev) {
      case 'INFO': return <IconCheckCircle size={12} />;
      case 'WARNING': return <IconAlertTriangle size={12} />;
      case 'CRITICAL':
      case 'ERROR': return <IconXCircle size={12} />;
      default: return <IconActivity size={12} />;
    }
  };

  return (
    <div className="card event-log-card" id="event-log-card">
      <div className="card-header">
        <div className="header-left">
          <h3 className="card-title">
            <IconList className="text-cyan" size={20} />
            Operational Event Stream Log
          </h3>
          <span className="item-count-pill font-mono">{eventsList.length} Events</span>
        </div>

        <div className="header-actions">
          {/* Severity filter */}
          <div className="filter-wrapper" style={{ display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
            <IconFilter size={14} className="text-muted" />
            <select
              className="select-industrial font-mono"
              value={filterSeverity}
              onChange={(e) => setFilterSeverity(e.target.value)}
              id="select-event-severity"
              aria-label="Filter events by severity"
              style={{ padding: '0.35rem 0.65rem', fontSize: '0.8rem' }}
            >
              <option value="ALL">All Severities</option>
              <option value="INFO">INFO Only</option>
              <option value="WARNING">WARNING Only</option>
              <option value="CRITICAL">CRITICAL Only</option>
            </select>
          </div>

          {/* Refresh button */}
          <button
            className="btn-industrial btn-sm btn-reset"
            onClick={onRefresh}
            title="Refresh event stream"
            id="btn-refresh-events"
          >
            <IconRefresh size={14} /> Refresh
          </button>
        </div>
      </div>

      <div className="card-body">
        {isLoading && eventsList.length === 0 ? (
          <div className="loading-state">
            <div className="spinner" aria-label="Loading event stream"></div>
            <p className="text-center text-muted font-mono">Polling operational event log stream...</p>
          </div>
        ) : filteredEvents.length === 0 ? (
          <div className="offline-state-message font-mono">
            <IconList size={20} className="text-muted" />
            <span>No operational events match the selected severity filter.</span>
          </div>
        ) : (
          <>
            {/* Desktop Table View */}
            <div className="event-table-container desktop-only">
              <table className="event-table" id="event-log-table">
                <thead>
                  <tr>
                    <th>TIME</th>
                    <th>SEVERITY</th>
                    <th>EVENT TYPE</th>
                    <th>ITEM ID</th>
                    <th>GATE</th>
                    <th>EVENT MESSAGE</th>
                  </tr>
                </thead>
                <tbody>
                  {filteredEvents.map((evt, idx) => {
                    const formattedTime = evt.timestampMs > 0
                      ? new Date(evt.timestampMs).toLocaleTimeString()
                      : 'N/A';

                    return (
                      <tr key={evt.eventId || `evt-${idx}`} className={`event-row ${getSeverityBadgeClass(evt.severity)}`}>
                        <td className="font-mono text-dim" style={{ whiteSpace: 'nowrap' }}>{formattedTime}</td>
                        <td>
                          <span className={`severity-badge ${getSeverityBadgeClass(evt.severity)}`}>
                            {getSeverityIcon(evt.severity)}
                            {evt.severity || 'INFO'}
                          </span>
                        </td>
                        <td>
                          <span className="event-type-tag">{evt.type || 'UNKNOWN'}</span>
                        </td>
                        <td className="font-mono text-cyan">
                          {evt.itemId && evt.itemId.trim() !== '' ? evt.itemId : '-'}
                        </td>
                        <td className="font-mono text-center">
                          {evt.gateId > 0 ? `#${evt.gateId}` : '-'}
                        </td>
                        <td className="event-msg-cell">{evt.message || '-'}</td>
                      </tr>
                    );
                  })}
                </tbody>
              </table>
            </div>

            {/* Mobile / Tablet Responsive Event Cards View */}
            <div className="mobile-event-card-list mobile-only">
              {filteredEvents.map((evt, idx) => {
                const formattedTime = evt.timestampMs > 0
                  ? new Date(evt.timestampMs).toLocaleTimeString()
                  : 'N/A';

                return (
                  <div key={evt.eventId || `m-evt-${idx}`} className={`mobile-event-card ${getSeverityBadgeClass(evt.severity)}`}>
                    <div className="m-evt-header">
                      <span className={`severity-badge ${getSeverityBadgeClass(evt.severity)}`}>
                        {getSeverityIcon(evt.severity)} {evt.severity}
                      </span>
                      <span className="event-type-tag">{evt.type}</span>
                      <span className="m-evt-time font-mono">{formattedTime}</span>
                    </div>

                    <div className="m-evt-body font-mono">
                      {evt.message}
                    </div>

                    <div className="m-evt-footer font-mono">
                      <span>Item: <strong className="text-cyan">{evt.itemId || '-'}</strong></span>
                      <span>Gate: <strong>{evt.gateId > 0 ? `#${evt.gateId}` : '-'}</strong></span>
                    </div>
                  </div>
                );
              })}
            </div>
          </>
        )}
      </div>
    </div>
  );
}

export default EventLog;
