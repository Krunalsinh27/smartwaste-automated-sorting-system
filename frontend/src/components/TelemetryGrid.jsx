import React from 'react';
import { TelemetryCard } from './TelemetryCard';
import { formatTimestamp } from '../utils/formatters';

/**
 * Responsive grid displaying real-time telemetry from C++ hardware sensors & actuators.
 */
export function TelemetryGrid({ telemetryData, isLoading, isError, errorMessage }) {
  if (isLoading) {
    return (
      <section className="section-container" id="telemetry-section">
        <h3 className="section-title">Hardware Telemetry Stream</h3>
        <div className="alert-box checking">Loading live telemetry stream...</div>
      </section>
    );
  }

  if (isError) {
    return (
      <section className="section-container" id="telemetry-section">
        <h3 className="section-title">Hardware Telemetry Stream</h3>
        <div className="alert-box disconnected">
          <strong>Telemetry Stream Error:</strong> {errorMessage || 'Telemetry endpoint unavailable.'}
        </div>
      </section>
    );
  }

  const {
    timestampMs = 0,
    conveyorSpeed = 0,
    motorRPM = 0,
    motorRunning = false,
    sensorSignal = 0,
    sensorTriggered = false,
    gateJamDetected = false,
  } = telemetryData || {};

  const sensorPct = Math.round(sensorSignal * 100);

  return (
    <section className="section-container" id="telemetry-section">
      <div className="section-header">
        <h3 className="section-title">Hardware Telemetry Stream</h3>
        <span className="live-indicator">● LIVE TELEMETRY</span>
      </div>

      <div className="metrics-grid">
        <TelemetryCard
          id="telem-conveyor"
          title="Conveyor Belt Speed"
          value={conveyorSpeed.toFixed(2)}
          unit="m/s"
          statusClass={conveyorSpeed > 0 ? 'positive' : 'normal'}
          subtext={conveyorSpeed > 0 ? 'Conveyor Active' : 'Conveyor Stopped'}
          icon="🏃"
        />

        <TelemetryCard
          id="telem-motor"
          title="Drive Motor Speed"
          value={motorRPM.toFixed(1)}
          unit="RPM"
          statusClass={motorRunning ? 'positive' : 'normal'}
          subtext={motorRunning ? 'Motor Drive Engaged' : 'Motor Powered Down'}
          icon="⚡"
        />

        <TelemetryCard
          id="telem-sensor"
          title="Proximity Sensor Signal"
          value={`${sensorPct}%`}
          unit=""
          statusClass={sensorTriggered ? 'warning' : 'normal'}
          subtext={sensorTriggered ? 'Item Trigger Active' : 'Clear Line-of-Sight'}
          icon="👁️"
        />

        <TelemetryCard
          id="telem-gate-jam"
          title="Diverter Gate Condition"
          value={gateJamDetected ? 'JAMMED' : 'NORMAL'}
          unit=""
          statusClass={gateJamDetected ? 'negative' : 'positive'}
          subtext={gateJamDetected ? 'Actuator obstruction!' : 'Gates Clear & Operational'}
          icon="🚧"
        />

        <TelemetryCard
          id="telem-timestamp"
          title="Telemetry Timestamp"
          value={formatTimestamp(timestampMs)}
          unit=""
          statusClass="normal"
          subtext={`Epoch: ${timestampMs} ms`}
          icon="🕒"
        />
      </div>
    </section>
  );
}
