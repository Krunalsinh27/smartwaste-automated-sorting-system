/**
 * SmartWaste Centralized API Client
 * Primary HTTP Client for communicating with the C++ REST API Backend.
 */

const API_BASE_URL = (import.meta.env.VITE_API_BASE_URL || 'http://127.0.0.1:8080').replace(/\/+$/, '');

export const getApiBaseUrl = () => API_BASE_URL;

/**
 * Generic request wrapper for C++ REST API calls.
 */
export async function requestApi(endpoint, options = {}, timeoutMs = 5000) {
  const controller = new AbortController();
  const timeoutId = setTimeout(() => controller.abort(), timeoutMs);

  const url = `${API_BASE_URL}${endpoint}`;
  const fetchOptions = {
    headers: {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
      ...options.headers,
    },
    signal: controller.signal,
    ...options,
  };

  try {
    const response = await fetch(url, fetchOptions);
    clearTimeout(timeoutId);

    const json = await response.json().catch(() => null);

    if (!response.ok) {
      const errorMsg = json?.error?.message || json?.message || `HTTP ${response.status} ${response.statusText}`;
      const errorCode = json?.error?.code || 'API_ERROR';
      return {
        success: false,
        error: errorMsg,
        code: errorCode,
        status: response.status,
      };
    }

    return {
      success: json?.success === true,
      message: json?.message || 'Operation succeeded',
      data: json?.data !== undefined ? json.data : json,
      status: response.status,
    };
  } catch (err) {
    clearTimeout(timeoutId);
    let errorMessage = 'Network request failed';
    if (err.name === 'AbortError') {
      errorMessage = `Request timed out after ${timeoutMs}ms`;
    } else if (err.message) {
      if (err.message.includes('Failed to fetch') || err.message.includes('NetworkError')) {
        errorMessage = 'SmartWaste C++ backend offline';
      } else {
        errorMessage = err.message;
      }
    }

    return {
      success: false,
      error: errorMessage,
      code: 'NETWORK_ERROR',
      status: 0,
    };
  }
}

/**
 * Check backend connectivity via GET /api/v1/ping
 */
export async function pingApi(timeoutMs = 3000) {
  const startTime = performance.now();
  const res = await requestApi('/api/v1/ping', { method: 'GET' }, timeoutMs);
  const endTime = performance.now();
  
  return {
    isOnline: res.success,
    message: res.message || (res.success ? 'SmartWaste API is running' : res.error),
    latencyMs: Math.round(endTime - startTime),
    timestamp: new Date().toLocaleTimeString(),
  };
}
