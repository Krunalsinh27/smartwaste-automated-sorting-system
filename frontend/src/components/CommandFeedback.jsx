import React from 'react';

/**
 * Feedback banner displaying execution status, loading state, or API transition errors.
 */
export function CommandFeedback({ commandState, onClear }) {
  const { isExecuting, feedbackMessage, isError, timestamp } = commandState;

  if (!feedbackMessage && !isExecuting) {
    return null;
  }

  let boxClass = 'feedback-info';
  if (isExecuting) boxClass = 'feedback-executing';
  else if (isError) boxClass = 'feedback-error';
  else boxClass = 'feedback-success';

  return (
    <div className={`command-feedback-banner ${boxClass}`} id="command-feedback-banner">
      <div className="feedback-content">
        {isExecuting && <span className="spinner-dot"></span>}
        <span className="feedback-text">{feedbackMessage}</span>
        {timestamp && <span className="feedback-time">[{timestamp}]</span>}
      </div>
      {!isExecuting && onClear && (
        <button className="btn-close-feedback" onClick={onClear} title="Dismiss">
          ✕
        </button>
      )}
    </div>
  );
}
