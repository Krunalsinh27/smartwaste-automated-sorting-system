/**
 * SortingResultCard Component
 * Displays the actual C++ SmartWasteEngine SortingResult object returned via REST API.
 * Visualizes SUCCESS (gate actuation) vs REJECTED (confidence threshold fallback) states.
 */
import React from 'react';
import { IconZap, IconCheckCircle, IconAlertTriangle, IconXCircle, IconTarget } from './Icons';

export function SortingResultCard({ result, isSubmitting, error }) {
  if (isSubmitting) {
    return (
      <div className="card sorting-result-card" id="sorting-result-card">
        <div className="card-header">
          <h4 className="card-title">
            <IconZap className="text-cyan" size={18} />
            C++ Engine Sorting Result
          </h4>
          <span className="card-badge">PROCESSING</span>
        </div>
        <div className="card-body">
          <div className="spinner"></div>
          <p className="text-center text-muted font-mono">Executing classification & pneumatic gate actuation...</p>
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="card sorting-result-card" id="sorting-result-card">
        <div className="card-header">
          <h4 className="card-title">
            <IconZap className="text-red" size={18} />
            Sorting Engine Result
          </h4>
          <span className="card-badge offline">ERROR</span>
        </div>
        <div className="card-body">
          <div className="offline-state-message">
            <IconXCircle size={24} className="text-red" />
            <div>
              <h4>Sorting Process Failed</h4>
              <p>{error}</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (!result) {
    return (
      <div className="card sorting-result-card" id="sorting-result-card">
        <div className="card-header">
          <h4 className="card-title">
            <IconZap className="text-muted" size={18} />
            C++ Engine Sorting Result
          </h4>
          <span className="card-badge font-mono">STANDBY</span>
        </div>
        <div className="card-body">
          <div className="empty-result-placeholder">
            <IconTarget size={36} className="text-muted" />
            <p>Enter waste payload details and click <strong>PROCESS WASTE ITEM</strong> to trigger C++ classification and pneumatic gate actuation.</p>
          </div>
        </div>
      </div>
    );
  }

  const {
    type = 'UNKNOWN',
    targetGate = 0,
    confidence = 0,
    status = 'UNKNOWN',
    message = '',
  } = result;

  const isSuccess = status === 'SUCCESS';
  const isRejected = status === 'REJECTED';

  const badgeClass = isSuccess
    ? 'res-badge-success'
    : isRejected
    ? 'res-badge-rejected'
    : 'res-badge-error';

  return (
    <div className={`card sorting-result-card ${isSuccess ? 'res-card-success' : 'res-card-rejected'}`} id="sorting-result-card">
      <div className="card-header">
        <h4 className="card-title">
          <IconZap className={isSuccess ? 'text-green' : 'text-amber'} size={18} />
          C++ Engine Sorting Result
        </h4>
        <span className={`result-status-pill ${badgeClass}`} id="sorting-status-pill">
          {isSuccess ? <IconCheckCircle size={14} /> : <IconAlertTriangle size={14} />}
          {isSuccess ? 'SUCCESS' : isRejected ? 'REJECTED' : status}
        </span>
      </div>

      <div className="card-body">
        <div className="result-details-grid">
          {/* Status */}
          <div className="result-detail-item">
            <span className="detail-label">Pipeline Action</span>
            <span className={`detail-val font-bold ${isSuccess ? 'text-success' : 'text-rejected'}`}>
              {status}
            </span>
          </div>

          {/* Waste Type */}
          <div className="result-detail-item">
            <span className="detail-label">Waste Type</span>
            <span className="detail-val font-mono text-cyan">{type}</span>
          </div>

          {/* Target Gate */}
          <div className="result-detail-item">
            <span className="detail-label">Target Diverter</span>
            <span className={`detail-val font-mono ${targetGate > 0 ? 'text-gate' : 'text-dim'}`}>
              {targetGate > 0 ? `Gate #${targetGate}` : 'Gate #0 (Bypass / Reject)'}
            </span>
          </div>

          {/* Confidence */}
          <div className="result-detail-item">
            <span className="detail-label">Confidence Score</span>
            <span className="detail-val font-mono">
              {(confidence * 100).toFixed(1)}% <span className="unit">({confidence.toFixed(2)})</span>
            </span>
          </div>
        </div>

        {/* Engine Response Message */}
        {message && (
          <div className="engine-message-box font-mono">
            <span className="msg-label">C++ Engine Message:</span> {message}
          </div>
        )}
      </div>
    </div>
  );
}

export default SortingResultCard;
