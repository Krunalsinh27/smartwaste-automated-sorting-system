import React from 'react';

const defaultProps = {
  size: 18,
  className: '',
  strokeWidth: 2,
};

export const IconLogo = ({ size = 22, className = '' }) => (
  <svg width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round" className={className}>
    <path d="M7 19H4.815a1.83 1.83 0 0 1-1.57-.91 1.78 1.78 0 0 1-.06-1.809l4.58-7.98" />
    <path d="M11 19h8.205a1.79 1.79 0 0 0 1.56-.91 1.78 1.78 0 0 0 .07-1.809l-4.58-7.98" />
    <path d="M9.1 4.58a1.78 1.78 0 0 1 1.58-.91h2.64a1.78 1.78 0 0 1 1.58.91l3.5 6.1" />
    <polyline points="7 19 9 22 11 19" />
    <polyline points="20 12 22 15 19 16" />
    <polyline points="5 10 2 10 3 7" />
  </svg>
);

export const IconPlay = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="currentColor" stroke="none" className={p.className}>
      <polygon points="5,3 19,12 5,21" />
    </svg>
  );
};

export const IconPause = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="currentColor" stroke="none" className={p.className}>
      <rect x="6" y="4" width="4" height="16" rx="1" />
      <rect x="14" y="4" width="4" height="16" rx="1" />
    </svg>
  );
};

export const IconResume = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <polygon points="5 3 19 12 5 21 5 3" fill="currentColor" />
    </svg>
  );
};

export const IconStop = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="currentColor" stroke="none" className={p.className}>
      <rect x="5" y="5" width="14" height="14" rx="2" />
    </svg>
  );
};

export const IconReset = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8" />
      <path d="M3 3v5h5" />
    </svg>
  );
};

export const IconAlertTriangle = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z" />
      <line x1="12" y1="9" x2="12" y2="13" />
      <line x1="12" y1="17" x2="12.01" y2="17" />
    </svg>
  );
};

export const IconShield = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
    </svg>
  );
};

export const IconActivity = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <polyline points="22 12 18 12 15 21 9 3 6 12 2 12" />
    </svg>
  );
};

export const IconCpu = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <rect x="4" y="4" width="16" height="16" rx="2" />
      <rect x="9" y="9" width="6" height="6" />
      <line x1="9" y1="1" x2="9" y2="4" />
      <line x1="15" y1="1" x2="15" y2="4" />
      <line x1="9" y1="20" x2="9" y2="23" />
      <line x1="15" y1="20" x2="15" y2="23" />
      <line x1="20" y1="9" x2="23" y2="9" />
      <line x1="20" y1="15" x2="23" y2="15" />
      <line x1="1" y1="9" x2="4" y2="9" />
      <line x1="1" y1="15" x2="4" y2="15" />
    </svg>
  );
};

export const IconBox = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z" />
      <polyline points="3.27 6.96 12 12.01 20.73 6.96" />
      <line x1="12" y1="22.08" x2="12" y2="12" />
    </svg>
  );
};

export const IconCheckCircle = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M22 11.08V12a10 10 0 1 1-5.93-9.14" />
      <polyline points="22 4 12 14.01 9 11.01" />
    </svg>
  );
};

export const IconXCircle = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <circle cx="12" cy="12" r="10" />
      <line x1="15" y1="9" x2="9" y2="15" />
      <line x1="9" y1="9" x2="15" y2="15" />
    </svg>
  );
};

export const IconZap = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2" />
    </svg>
  );
};

export const IconTarget = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <circle cx="12" cy="12" r="10" />
      <circle cx="12" cy="12" r="6" />
      <circle cx="12" cy="12" r="2" />
    </svg>
  );
};

export const IconGauge = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M12 15a3 3 0 1 0 0-6 3 3 0 0 0 0 6z" />
      <path d="M12 2A10 10 0 0 0 2 12a9.9 9.9 0 0 0 2.83 7" />
      <path d="M12 2a10 10 0 0 1 10 10 9.9 9.9 0 0 1-2.83 7" />
      <path d="M12 12l4-4" />
    </svg>
  );
};

export const IconGear = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <circle cx="12" cy="12" r="3" />
      <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z" />
    </svg>
  );
};

export const IconRadio = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <circle cx="12" cy="12" r="2" />
      <path d="M16.24 7.76a6 6 0 0 1 0 8.49" />
      <path d="M7.76 16.24a6 6 0 0 1 0-8.49" />
      <path d="M19.07 4.93a10 10 0 0 1 0 14.14" />
      <path d="M4.93 19.07a10 10 0 0 1 0-14.14" />
    </svg>
  );
};

export const IconBarrier = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M3 18v3" />
      <path d="M21 18v3" />
      <path d="M3 6v3" />
      <path d="M21 6v3" />
      <rect x="3" y="9" width="18" height="9" rx="1" />
      <path d="M7 9l5 9" />
      <path d="M13 9l5 9" />
    </svg>
  );
};

export const IconHeartPulse = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M19 14c1.49-1.46 3-3.21 3-5.5A5.5 5.5 0 0 0 16.5 3c-1.76 0-3 .5-4.5 2-1.5-1.5-2.74-2-4.5-2A5.5 5.5 0 0 0 2 8.5c0 2.3 1.5 4.05 3 5.5l7 7Z" />
      <path d="M3.22 12H9.5l.5-1 2 4.5 2-7 1.5 3.5h5.27" />
    </svg>
  );
};

export const IconList = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <line x1="8" y1="6" x2="21" y2="6" />
      <line x1="8" y1="12" x2="21" y2="12" />
      <line x1="8" y1="18" x2="21" y2="18" />
      <line x1="3" y1="6" x2="3.01" y2="6" />
      <line x1="3" y1="12" x2="3.01" y2="12" />
      <line x1="3" y1="18" x2="3.01" y2="18" />
    </svg>
  );
};

export const IconRefresh = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M21.5 2v6h-6" />
      <path d="M21.34 15.57a10 10 0 1 1-.57-8.38l5.67-5.67" />
    </svg>
  );
};

export const IconFilter = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <polygon points="22 3 2 3 10 12.46 10 19 14 21 14 12.46 22 3" />
    </svg>
  );
};

export const IconDice = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <rect x="3" y="3" width="18" height="18" rx="2" ry="2" />
      <circle cx="8.5" cy="8.5" r="1.5" fill="currentColor" />
      <circle cx="15.5" cy="8.5" r="1.5" fill="currentColor" />
      <circle cx="12" cy="12" r="1.5" fill="currentColor" />
      <circle cx="8.5" cy="15.5" r="1.5" fill="currentColor" />
      <circle cx="15.5" cy="15.5" r="1.5" fill="currentColor" />
    </svg>
  );
};

export const IconServer = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <rect x="2" y="2" width="20" height="8" rx="2" ry="2" />
      <rect x="2" y="14" width="20" height="8" rx="2" ry="2" />
      <line x1="6" y1="6" x2="6.01" y2="6" />
      <line x1="6" y1="18" x2="6.01" y2="18" />
    </svg>
  );
};

export const IconWifi = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <path d="M5 12.55a11 11 0 0 1 14.08 0" />
      <path d="M1.42 9a16 16 0 0 1 21.16 0" />
      <path d="M8.53 16.11a6 6 0 0 1 6.95 0" />
      <line x1="12" y1="20" x2="12.01" y2="20" />
    </svg>
  );
};

export const IconWifiOff = (props) => {
  const p = { ...defaultProps, ...props };
  return (
    <svg width={p.size} height={p.size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={p.strokeWidth} strokeLinecap="round" strokeLinejoin="round" className={p.className}>
      <line x1="1" y1="1" x2="23" y2="23" />
      <path d="M16.72 11.06A10.94 10.94 0 0 1 19 12.55" />
      <path d="M5 12.55a10.94 10.94 0 0 1 5.17-2.39" />
      <path d="M10.71 5.05A16 16 0 0 1 22.58 9" />
      <path d="M1.42 9a15.91 15.91 0 0 1 4.7-2.88" />
      <path d="M8.53 16.11a6 6 0 0 1 6.95 0" />
      <line x1="12" y1="20" x2="12.01" y2="20" />
    </svg>
  );
};
