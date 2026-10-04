/**
 * ApiStatus Component
 * Displays system connection status: API ONLINE / API OFFLINE, backend host info (127.0.0.1:8080), latency & LIVE DATA stream indicator.
 */
import React from 'react';
import { IconWifi, IconWifiOff, IconServer, IconActivity } from './Icons';

export function ApiStatus({ apiStatus }) {
  const { isOnline, latencyMs, message, lastChecked } = apiStatus;

  return (
    <div className={`api-status-badge ${isOnline ? 'online' : 'offline'}`} id="api-status-badge" aria-label="System Connection Status">
      <div className={`status-indicator ${isOnline ? 'online' : 'offline'}`}>
        <span className={`pulse-dot ${isOnline ? 'online' : 'offline'}`} aria-hidden="true"></span>
        {isOnline ? <IconWifi size={15} /> : <IconWifiOff size={15} />}
        <span>{isOnline ? 'ONLINE' : 'OFFLINE'}</span>
      </div>

      <div className="backend-host-pill font-mono">
        <IconServer size={12} style={{ display: 'inline', marginRight: '4px', verticalAlign: 'middle' }} />
        127.0.0.1:8080
      </div>

      {isOnline ? (
        <div className="status-details font-mono">
          <span className="live-data-pill">
            <IconActivity size={12} /> LIVE
          </span>
          <span className="latency-pill">{latencyMs} ms</span>
        </div>
      ) : (
        <div className="status-details error font-mono">
          <span className="text-red font-bold">{message || 'Unreachable'}</span>
        </div>
      )}
    </div>
  );
}

export default ApiStatus;
