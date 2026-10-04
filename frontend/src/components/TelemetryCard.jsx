/**
 * TelemetryCard Component
 * Displays 4 compact telemetry indicators: Conveyor Speed, Motor RPM, Optical Sensor, Gate Status, and Last Updated timestamp.
 */
import React from 'react';
import {
  IconActivity,
  IconGauge,
  IconGear,
  IconRadio,
  IconBarrier,
  IconAlertTriangle
} from './Icons';

export function TelemetryCard({ telemetryData, isOnline, isLoading, error }) {
  if (!isOnline || error) {
    return (
      <div className="card telemetry-card" id="telemetry-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconActivity className="text-muted" size={20} />
            Real-Time Hardware Telemetry
          </h3>
          <span className="card-badge offline">OFFLINE</span>
        </div>
        <div className="card-body">
          <div className="offline-state-message">
            <IconAlertTriangle size={24} className="text-amber" />
            <div>
              <h4>TELEMETRY UNAVAILABLE</h4>
              <p>{error || 'SmartWaste backend is currently offline or unreachable.'}</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (isLoading && !telemetryData) {
    return (
      <div className="card telemetry-card" id="telemetry-card">
        <div className="card-header">
          <h3 className="card-title">
            <IconActivity className="text-cyan" size={20} />
            Real-Time Hardware Telemetry
          </h3>
          <span className="card-badge">Streaming...</span>
        </div>
        <div className="card-body">
          <div className="spinner" aria-label="Loading telemetry stream"></div>
          <p className="text-center text-muted font-mono">Polling hardware telemetry snapshot...</p>
        </div>
      </div>
    );
  }

  const {
    conveyorSpeed = 0,
    motorRPM = 0,
    sensorSignal = 0,
    sensorTriggered = false,
    gateJamDetected = false,
    timestampMs = 0,
  } = telemetryData || {};

  const formattedTime = timestampMs > 0
    ? new Date(timestampMs).toLocaleTimeString()
    : 'N/A';

  return (
    <div className="card telemetry-card" id="telemetry-card">
      <div className="card-header">
        <h3 className="card-title">
          <IconActivity className="text-cyan" size={20} />
          Real-Time Hardware Telemetry
        </h3>
        <span className="live-indicator font-mono">LIVE STREAM</span>
      </div>

      <div className="card-body">
        <div className="telemetry-compact-grid">
          {/* 1. Conveyor Speed */}
          <div className="tele-compact-item">
            <div className="tele-item-header">
              <IconGauge size={14} />
              <span className="tele-label">CONVEYOR SPEED</span>
            </div>
            <div className="tele-value-display font-mono">
              {typeof conveyorSpeed === 'number' ? conveyorSpeed.toFixed(1) : conveyorSpeed} <span className="tele-unit">m/s</span>
            </div>
          </div>

          {/* 2. Motor RPM */}
          <div className="tele-compact-item">
            <div className="tele-item-header">
              <IconGear size={14} />
              <span className="tele-label">MOTOR SPEED</span>
            </div>
            <div className="tele-value-display font-mono">
              {typeof motorRPM === 'number' ? Math.round(motorRPM) : motorRPM} <span className="tele-unit">RPM</span>
            </div>
          </div>

          {/* 3. Optical Sensor */}
          <div className="tele-compact-item">
            <div className="tele-item-header">
              <IconRadio size={14} />
              <span className="tele-label">OPTICAL SENSOR</span>
            </div>
            <div className={`tele-value-display font-mono ${sensorTriggered ? 'warning' : 'active'}`}>
              {sensorTriggered ? 'TRIGGERED' : 'CLEAR'}
            </div>
          </div>

          {/* 4. Gate Status */}
          <div className="tele-compact-item">
            <div className="tele-item-header">
              <IconBarrier size={14} />
              <span className="tele-label">DIVERTER GATES</span>
            </div>
            <div className={`tele-value-display font-mono ${gateJamDetected ? 'fault' : 'active'}`}>
              {gateJamDetected ? 'JAMMED' : 'READY'}
            </div>
          </div>
        </div>

        {/* Timestamp Footer */}
        <div className="tele-timestamp-footer font-mono">
          <span className="ts-label">Last Telemetry Snapshot:</span>
          <span className="ts-val font-bold text-cyan">{formattedTime}</span>
        </div>
      </div>
    </div>
  );
}

export default TelemetryCard;
