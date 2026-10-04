/**
 * Formatting utility functions for industrial dashboard UI.
 */

export function formatTimestamp(val) {
  if (!val) return '--:--:--';
  try {
    const d = typeof val === 'number' ? new Date(val) : new Date(val);
    if (isNaN(d.getTime())) return String(val);

    return d.toLocaleTimeString('en-US', {
      hour12: false,
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
    }) + '.' + String(d.getMilliseconds()).padStart(3, '0');
  } catch {
    return String(val);
  }
}

export function formatResponseTime(ms) {
  if (ms === undefined || ms === null || ms < 0) return 'N/A';
  return `${ms} ms`;
}
