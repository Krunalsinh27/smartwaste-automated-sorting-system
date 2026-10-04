import React, { useState, useMemo } from 'react';
import { EventStream } from './EventStream';
import { formatTimestamp } from '../utils/formatters';

const EVENT_TYPES = [
  'ALL',
  'MACHINE_STARTED',
  'MACHINE_STOPPED',
  'EMERGENCY_STOP',
  'MACHINE_RESET',
  'ITEM_DETECTED',
  'ITEM_CLASSIFIED',
  'ITEM_SORTED',
  'ITEM_REJECTED',
  'GATE_ACTUATED',
  'GATE_JAMMED',
  'GATE_UNAVAILABLE',
  'HEALTH_WARNING',
  'HEALTH_FAULT',
];

const SEVERITY_LEVELS = ['ALL', 'DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'];

export function EventHistory({ events = [], isLoading, isError, errorMessage, onRefresh, isConnected }) {
  const [selectedType, setSelectedType] = useState('ALL');
  const [selectedSeverity, setSelectedSeverity] = useState('ALL');
  const [searchTerm, setSearchTerm] = useState('');

  // Filter events client-side based on actual backend fields
  const filteredEvents = useMemo(() => {
    return events.filter((evt) => {
      // Type filter
      if (selectedType !== 'ALL' && evt.type !== selectedType) {
        return false;
      }
      // Severity filter
      if (selectedSeverity !== 'ALL' && evt.severity !== selectedSeverity) {
        return false;
      }
      // Search term filter (matches itemId, message, eventId, wasteType)
      if (searchTerm.trim()) {
        const query = searchTerm.toLowerCase();
        const itemMatch = evt.itemId && evt.itemId.toLowerCase().includes(query);
        const msgMatch = evt.message && evt.message.toLowerCase().includes(query);
        const idMatch = evt.eventId && evt.eventId.toLowerCase().includes(query);
        const typeMatch = evt.type && evt.type.toLowerCase().includes(query);
        if (!itemMatch && !msgMatch && !idMatch && !typeMatch) {
          return false;
        }
      }
      return true;
    });
  }, [events, selectedType, selectedSeverity, searchTerm]);

  // Calculate critical/warning count from actual backend severity strings
  const criticalCount = useMemo(() => {
    return events.filter(
      (e) => e.severity === 'WARNING' || e.severity === 'ERROR' || e.severity === 'CRITICAL'
    ).length;
  }, [events]);

  const handleResetFilters = () => {
    setSelectedType('ALL');
    setSelectedSeverity('ALL');
    setSearchTerm('');
  };

  const getSeverityBadgeClass = (severity) => {
    switch (severity) {
      case 'CRITICAL':
      case 'ERROR':
        return 'state-critical';
      case 'WARNING':
      case 'WARN':
        return 'state-warning';
      case 'INFO':
        return 'state-neutral';
      case 'DEBUG':
      default:
        return 'state-offline';
    }
  };

  return (
    <section className="section-container" id="event-history-section">
      <EventStream
        totalLoaded={events.length}
        totalVisible={filteredEvents.length}
        criticalCount={criticalCount}
        isConnected={isConnected && !isError}
        isRefreshing={isLoading}
        onRefresh={onRefresh}
      />

      {/* Filter Toolbar */}
      <div className="event-filters-toolbar">
        {/* Type Filter */}
        <div className="filter-group">
          <label className="filter-label" htmlFor="filter-event-type">Event Category</label>
          <select
            id="filter-event-type"
            className="form-select filter-select"
            value={selectedType}
            onChange={(e) => setSelectedType(e.target.value)}
          >
            <option value="ALL">All Categories</option>
            {EVENT_TYPES.filter((t) => t !== 'ALL').map((t) => (
              <option key={t} value={t}>
                {t}
              </option>
            ))}
          </select>
        </div>

        {/* Severity Filter */}
        <div className="filter-group">
          <label className="filter-label" htmlFor="filter-severity">Severity Level</label>
          <select
            id="filter-severity"
            className="form-select filter-select"
            value={selectedSeverity}
            onChange={(e) => setSelectedSeverity(e.target.value)}
          >
            <option value="ALL">All Severities</option>
            {SEVERITY_LEVELS.filter((s) => s !== 'ALL').map((s) => (
              <option key={s} value={s}>
                {s}
              </option>
            ))}
          </select>
        </div>

        {/* Item ID / Text Search */}
        <div className="filter-group filter-search-group">
          <label className="filter-label" htmlFor="search-item-id">Item ID / Message Search</label>
          <input
            type="text"
            id="search-item-id"
            className="form-input filter-input"
            placeholder="Filter by Item ID, Event ID, or text..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
          />
        </div>

        {/* Reset Filters */}
        <div className="filter-group filter-action-group">
          <label className="filter-label">&nbsp;</label>
          <button className="btn-industrial btn-reset-filters" onClick={handleResetFilters}>
            Clear Filters
          </button>
        </div>
      </div>

      {/* Main Table / State Container */}
      {isError && (
        <div className="alert-box disconnected" style={{ marginTop: '1rem' }}>
          <strong>EVENT STREAM OFFLINE:</strong> {errorMessage || 'Unable to retrieve operational events stream.'}
          <div style={{ marginTop: '6px' }}>
            Please check that the SmartWaste C++ server is running.
          </div>
        </div>
      )}

      {!isError && isLoading && events.length === 0 && (
        <div className="alert-box checking" style={{ marginTop: '1rem' }}>
          <span className="spinner-dot"></span> Fetching operational events from backend...
        </div>
      )}

      {!isError && filteredEvents.length === 0 && events.length > 0 && (
        <div className="alert-box checking" style={{ marginTop: '1rem' }}>
          No events match the active filter criteria. Try clearing filters.
        </div>
      )}

      {!isError && events.length === 0 && !isLoading && (
        <div className="result-placeholder-text" style={{ marginTop: '1rem' }}>
          NO OPERATIONAL EVENTS RECORDED. Waiting for machine activity...
        </div>
      )}

      {/* Responsive Event Table */}
      {!isError && filteredEvents.length > 0 && (
        <div className="table-responsive-wrapper" style={{ marginTop: '1.25rem' }}>
          <table className="industrial-table" id="events-table">
            <thead>
              <tr>
                <th>Time</th>
                <th>Event ID</th>
                <th>Type</th>
                <th>Severity</th>
                <th>Message</th>
                <th>Item ID</th>
                <th>Waste Type</th>
                <th>Gate</th>
              </tr>
            </thead>
            <tbody>
              {filteredEvents.map((evt, idx) => (
                <tr key={evt.eventId || `evt-${idx}`}>
                  <td className="table-time-cell">{formatTimestamp(evt.timestampMs)}</td>
                  <td className="table-mono-cell">{evt.eventId || '--'}</td>
                  <td>
                    <span className="event-type-tag">{evt.type}</span>
                  </td>
                  <td>
                    <span className={`state-badge ${getSeverityBadgeClass(evt.severity)}`} style={{ fontSize: '0.75rem', padding: '2px 8px' }}>
                      {evt.severity}
                    </span>
                  </td>
                  <td className="table-message-cell">{evt.message}</td>
                  <td className="table-mono-cell">{evt.itemId ? evt.itemId : '--'}</td>
                  <td>{evt.wasteType && evt.wasteType !== 'UNKNOWN' ? evt.wasteType : '--'}</td>
                  <td>{evt.gateId > 0 ? `#${evt.gateId}` : '--'}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </section>
  );
}
