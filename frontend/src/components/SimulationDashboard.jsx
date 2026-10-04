import React, { useState } from 'react';
import { useSimulation } from '../hooks/useSimulation';

/**
 * Scenario presets for quick industrial batch simulation testing.
 */
const SCENARIO_PRESETS = [
  {
    id: 'STANDARD_MIXED',
    name: 'Standard Mixed Waste Batch',
    description: '10 balanced items across Plastic, Metal, Paper, Glass, and Other categories with high classifier confidence.',
    itemCount: 10,
    badge: '10 ITEMS',
    generate: () => [
      { id: 'SIM-PL-001', type: 'PLASTIC', confidence: 0.94, gateId: 1, boundingBox: { x: 10, y: 10, width: 120, height: 120 } },
      { id: 'SIM-MT-002', type: 'METAL', confidence: 0.91, gateId: 2, boundingBox: { x: 140, y: 10, width: 100, height: 100 } },
      { id: 'SIM-PP-003', type: 'PAPER', confidence: 0.88, gateId: 3, boundingBox: { x: 250, y: 10, width: 150, height: 110 } },
      { id: 'SIM-GL-004', type: 'GLASS', confidence: 0.96, gateId: 4, boundingBox: { x: 410, y: 10, width: 90, height: 130 } },
      { id: 'SIM-OT-005', type: 'OTHER', confidence: 0.82, gateId: 5, boundingBox: { x: 510, y: 10, width: 80, height: 80 } },
      { id: 'SIM-PL-006', type: 'PLASTIC', confidence: 0.95, gateId: 1, boundingBox: { x: 15, y: 140, width: 110, height: 110 } },
      { id: 'SIM-PL-007', type: 'PLASTIC', confidence: 0.89, gateId: 1, boundingBox: { x: 130, y: 140, width: 100, height: 105 } },
      { id: 'SIM-MT-008', type: 'METAL', confidence: 0.93, gateId: 2, boundingBox: { x: 240, y: 140, width: 95, height: 95 } },
      { id: 'SIM-PP-009', type: 'PAPER', confidence: 0.87, gateId: 3, boundingBox: { x: 345, y: 140, width: 140, height: 100 } },
      { id: 'SIM-GL-010', type: 'GLASS', confidence: 0.92, gateId: 4, boundingBox: { x: 495, y: 140, width: 85, height: 125 } },
    ],
  },
  {
    id: 'HIGH_PLASTIC',
    name: 'High-Volume Plastic Run',
    description: '20 high-density PET and HDPE plastic container items target Gate 1.',
    itemCount: 20,
    badge: '20 ITEMS',
    generate: () => {
      const items = [];
      const types = ['PLASTIC', 'PLASTIC', 'PLASTIC', 'PLASTIC', 'METAL'];
      for (let i = 1; i <= 20; i++) {
        const type = types[i % types.length];
        const gateId = type === 'PLASTIC' ? 1 : 2;
        items.push({
          id: `SIM-PET-${String(i).padStart(3, '0')}`,
          type,
          confidence: 0.90 + (i % 8) * 0.01,
          gateId,
          boundingBox: { x: (i * 30) % 600, y: (i * 20) % 400, width: 100, height: 100 },
        });
      }
      return items;
    },
  },
  {
    id: 'EDGE_CASES',
    name: 'Low Confidence & Rejection Run',
    description: '12 unclassified or low-confidence (<0.50) items to verify engine fallback rejections.',
    itemCount: 12,
    badge: 'REJECTION TEST',
    generate: () => {
      const items = [];
      const types = ['UNKNOWN', 'OTHER', 'PLASTIC', 'GLASS'];
      for (let i = 1; i <= 12; i++) {
        const type = types[i % types.length];
        // Mix low confidence items that trigger rejections
        const confidence = i % 2 === 0 ? 0.35 : 0.45;
        items.push({
          id: `SIM-REJ-${String(i).padStart(3, '0')}`,
          type,
          confidence,
          gateId: 0,
          boundingBox: { x: 50 * i, y: 50, width: 80, height: 80 },
        });
      }
      return items;
    },
  },
  {
    id: 'STRESS_TEST',
    name: 'Industrial Throughput Stress Run',
    description: '50 continuous waste items simulating rapid sorting conveyor stream.',
    itemCount: 50,
    badge: '50 ITEMS STRESS',
    generate: () => {
      const types = ['PLASTIC', 'METAL', 'PAPER', 'GLASS', 'OTHER'];
      const items = [];
      for (let i = 1; i <= 50; i++) {
        const type = types[i % types.length];
        const gateId = (i % 5) + 1;
        items.push({
          id: `SIM-STR-${String(i).padStart(3, '0')}`,
          type,
          confidence: 0.80 + (i % 18) * 0.01,
          gateId,
          boundingBox: { x: (i * 15) % 600, y: (i * 25) % 400, width: 90, height: 90 },
        });
      }
      return items;
    },
  },
];

export function SimulationDashboard() {
  const {
    isSimulating,
    simulationResult,
    simulationError,
    lastRunTime,
    executeSimulation,
    clearResults,
  } = useSimulation();

  const [selectedScenarioId, setSelectedScenarioId] = useState('STANDARD_MIXED');
  const [customItemCount, setCustomItemCount] = useState(15);
  const [customPrimaryType, setCustomPrimaryType] = useState('MIXED');
  const [activeTab, setActiveTab] = useState('PRESET'); // 'PRESET' | 'CUSTOM'

  const selectedPreset = SCENARIO_PRESETS.find((p) => p.id === selectedScenarioId) || SCENARIO_PRESETS[0];

  const handleRunPreset = async () => {
    const items = selectedPreset.generate();
    await executeSimulation(items);
  };

  const handleRunCustom = async () => {
    const types = customPrimaryType === 'MIXED' 
      ? ['PLASTIC', 'METAL', 'PAPER', 'GLASS', 'OTHER']
      : [customPrimaryType];

    const items = [];
    for (let i = 1; i <= customItemCount; i++) {
      const type = types[(i - 1) % types.length];
      const gateId = type === 'PLASTIC' ? 1 : type === 'METAL' ? 2 : type === 'PAPER' ? 3 : type === 'GLASS' ? 4 : 5;
      items.push({
        id: `SIM-CUST-${String(i).padStart(3, '0')}`,
        type,
        confidence: 0.88 + (i % 10) * 0.01,
        gateId,
        boundingBox: { x: (i * 20) % 500, y: (i * 30) % 350, width: 100, height: 100 },
      });
    }

    await executeSimulation(items);
  };

  const calculateSuccessRate = (stats) => {
    if (!stats || stats.totalItems === 0) return 0;
    return Math.round((stats.successfulItems / stats.totalItems) * 100);
  };

  return (
    <div className="industrial-card simulation-dashboard-card">
      <div className="card-header border-b flex justify-between items-center pb-3 mb-4">
        <div className="flex items-center space-x-3">
          <div className="header-icon-box bg-purple-900/40 text-purple-400 p-2 rounded">
            ⚡
          </div>
          <div>
            <h2 className="text-xl font-bold tracking-wide text-gray-100">
              AUTOMATED BATCH SIMULATION & SCENARIOS
            </h2>
            <p className="text-xs text-gray-400">
              Run automated item batch scenarios through the C++ SmartWaste engine hardware pipeline
            </p>
          </div>
        </div>

        <div className="flex items-center space-x-2">
          <button
            type="button"
            className={`px-3 py-1 text-xs font-semibold rounded ${
              activeTab === 'PRESET' ? 'bg-purple-600 text-white' : 'bg-gray-800 text-gray-400 hover:text-white'
            }`}
            onClick={() => setActiveTab('PRESET')}
          >
            PRESET SCENARIOS
          </button>
          <button
            type="button"
            className={`px-3 py-1 text-xs font-semibold rounded ${
              activeTab === 'CUSTOM' ? 'bg-purple-600 text-white' : 'bg-gray-800 text-gray-400 hover:text-white'
            }`}
            onClick={() => setActiveTab('CUSTOM')}
          >
            CUSTOM GENERATOR
          </button>
        </div>
      </div>

      {/* ERROR ALERT */}
      {simulationError && (
        <div className="bg-red-900/40 border border-red-500/50 text-red-200 p-3 rounded text-sm mb-4 flex items-center justify-between">
          <span>⚠️ {simulationError}</span>
          <button
            onClick={() => clearResults()}
            className="text-xs bg-red-800/60 hover:bg-red-700 text-white px-2 py-1 rounded"
          >
            Dismiss
          </button>
        </div>
      )}

      {/* CONFIGURATION SECTION */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-6">
        {activeTab === 'PRESET' ? (
          <div className="md:col-span-2 space-y-3">
            <label className="block text-xs font-bold text-gray-400 uppercase tracking-wider">
              Select Preset Scenario:
            </label>
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
              {SCENARIO_PRESETS.map((preset) => (
                <div
                  key={preset.id}
                  onClick={() => setSelectedScenarioId(preset.id)}
                  className={`p-3 rounded border cursor-pointer transition-all ${
                    selectedScenarioId === preset.id
                      ? 'bg-purple-950/60 border-purple-500 shadow-md shadow-purple-900/20'
                      : 'bg-gray-900/60 border-gray-800 hover:border-gray-700'
                  }`}
                >
                  <div className="flex justify-between items-center mb-1">
                    <span className="font-semibold text-sm text-gray-200">{preset.name}</span>
                    <span className="text-[10px] font-mono bg-purple-900/60 text-purple-300 px-1.5 py-0.5 rounded">
                      {preset.badge}
                    </span>
                  </div>
                  <p className="text-xs text-gray-400 line-clamp-2">{preset.description}</p>
                </div>
              ))}
            </div>
          </div>
        ) : (
          <div className="md:col-span-2 bg-gray-900/60 p-4 rounded border border-gray-800 space-y-4">
            <h3 className="text-sm font-bold text-purple-400 uppercase tracking-wider">
              Custom Batch Configuration
            </h3>
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div>
                <label className="block text-xs text-gray-400 mb-1">
                  Item Count: <span className="font-mono text-purple-300 font-bold">{customItemCount} items</span>
                </label>
                <input
                  type="range"
                  min="1"
                  max="100"
                  value={customItemCount}
                  onChange={(e) => setCustomItemCount(Number(e.target.value))}
                  className="w-full accent-purple-500 bg-gray-800"
                />
              </div>

              <div>
                <label className="block text-xs text-gray-400 mb-1">Target Primary Waste Type:</label>
                <select
                  value={customPrimaryType}
                  onChange={(e) => setCustomPrimaryType(e.target.value)}
                  className="w-full bg-gray-950 border border-gray-800 text-gray-200 text-xs rounded p-2 focus:border-purple-500 outline-none"
                >
                  <option value="MIXED">BALANCED MIXED STREAM</option>
                  <option value="PLASTIC">PLASTIC DOMINANT</option>
                  <option value="METAL">METAL HEAVY</option>
                  <option value="PAPER">PAPER & FIBER</option>
                  <option value="GLASS">GLASS CONTAINERS</option>
                </select>
              </div>
            </div>
            <p className="text-xs text-gray-500">
              Generates a custom sequence of simulated items with bounding boxes and confidence levels for hardware testing.
            </p>
          </div>
        )}

        {/* EXECUTION TRIGGER PANEL */}
        <div className="bg-gray-900/80 p-4 rounded border border-gray-800 flex flex-col justify-between">
          <div>
            <span className="text-xs font-bold text-gray-400 uppercase tracking-wider block mb-2">
              Batch Execution Controls
            </span>
            <div className="text-xs text-gray-400 space-y-1 mb-4">
              <div>
                Target Endpoint: <span className="font-mono text-gray-300">POST /api/v1/simulation/run</span>
              </div>
              <div>
                Selected Batch Size:{' '}
                <span className="font-mono text-purple-400 font-semibold">
                  {activeTab === 'PRESET' ? selectedPreset.itemCount : customItemCount} items
                </span>
              </div>
            </div>
          </div>

          <button
            type="button"
            disabled={isSimulating}
            onClick={activeTab === 'PRESET' ? handleRunPreset : handleRunCustom}
            className={`w-full py-3 px-4 rounded font-bold text-xs uppercase tracking-wider transition-all flex items-center justify-center space-x-2 shadow-lg ${
              isSimulating
                ? 'bg-purple-900/50 text-purple-300 cursor-not-allowed border border-purple-700/50'
                : 'bg-purple-600 hover:bg-purple-500 text-white shadow-purple-900/40 border border-purple-400/30'
            }`}
          >
            {isSimulating ? (
              <>
                <svg className="animate-spin h-4 w-4 text-purple-300" fill="none" viewBox="0 0 24 24">
                  <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4" />
                  <path
                    className="opacity-75"
                    fill="currentColor"
                    d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
                  />
                </svg>
                <span>RUNNING SIMULATION...</span>
              </>
            ) : (
              <span>⚡ RUN BATCH SIMULATION</span>
            )}
          </button>
        </div>
      </div>

      {/* RESULTS DISPLAY SECTION */}
      {simulationResult && (
        <div className="mt-6 border-t border-gray-800 pt-6 animate-fadeIn">
          <div className="flex justify-between items-center mb-4">
            <div className="flex items-center space-x-2">
              <span className="h-2.5 w-2.5 rounded-full bg-green-500 animate-pulse"></span>
              <h3 className="text-base font-bold text-gray-200 uppercase tracking-wide">
                Simulation Performance Report
              </h3>
              {lastRunTime && (
                <span className="text-xs text-gray-400 font-mono bg-gray-900 px-2 py-0.5 rounded border border-gray-800">
                  Executed at {lastRunTime}
                </span>
              )}
            </div>

            <button
              onClick={clearResults}
              className="text-xs text-gray-400 hover:text-white bg-gray-900 hover:bg-gray-800 border border-gray-800 px-2.5 py-1 rounded"
            >
              Clear Report
            </button>
          </div>

          {/* OVERVIEW METRICS GRID */}
          <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-6 gap-3 mb-4">
            <div className="bg-gray-950 p-3 rounded border border-gray-800 text-center">
              <span className="text-[10px] text-gray-400 uppercase font-semibold block">Total Items</span>
              <span className="text-xl font-bold font-mono text-gray-100">{simulationResult.totalItems}</span>
            </div>

            <div className="bg-green-950/40 p-3 rounded border border-green-900/50 text-center">
              <span className="text-[10px] text-green-400 uppercase font-semibold block">Successful</span>
              <span className="text-xl font-bold font-mono text-green-300">
                {simulationResult.successfulItems}
              </span>
            </div>

            <div className="bg-amber-950/40 p-3 rounded border border-amber-900/50 text-center">
              <span className="text-[10px] text-amber-400 uppercase font-semibold block">Rejected</span>
              <span className="text-xl font-bold font-mono text-amber-300">
                {simulationResult.rejectedItems}
              </span>
            </div>

            <div className="bg-blue-950/40 p-3 rounded border border-blue-900/50 text-center">
              <span className="text-[10px] text-blue-400 uppercase font-semibold block">Gate Actuations</span>
              <span className="text-xl font-bold font-mono text-blue-300">
                {simulationResult.totalGateActuations}
              </span>
            </div>

            <div className="bg-purple-950/40 p-3 rounded border border-purple-900/50 text-center">
              <span className="text-[10px] text-purple-400 uppercase font-semibold block">Health Checks</span>
              <span className="text-xl font-bold font-mono text-purple-300">
                {simulationResult.healthChecksPerformed}
              </span>
            </div>

            <div className="bg-teal-950/40 p-3 rounded border border-teal-900/50 text-center">
              <span className="text-[10px] text-teal-400 uppercase font-semibold block">Success Rate</span>
              <span className="text-xl font-bold font-mono text-teal-300">
                {calculateSuccessRate(simulationResult)}%
              </span>
            </div>
          </div>

          {/* HEALTH BREAKDOWN BAR & PROGRESS */}
          <div className="bg-gray-950 p-4 rounded border border-gray-800 grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
              <span className="text-xs font-bold text-gray-400 uppercase block mb-2">
                Batch Sorting Accuracy
              </span>
              <div className="w-full bg-gray-900 h-3 rounded-full overflow-hidden flex border border-gray-800">
                <div
                  className="bg-green-500 h-full transition-all duration-500"
                  style={{
                    width: `${
                      simulationResult.totalItems > 0
                        ? (simulationResult.successfulItems / simulationResult.totalItems) * 100
                        : 0
                    }%`,
                  }}
                  title={`Successful: ${simulationResult.successfulItems}`}
                ></div>
                <div
                  className="bg-amber-500 h-full transition-all duration-500"
                  style={{
                    width: `${
                      simulationResult.totalItems > 0
                        ? (simulationResult.rejectedItems / simulationResult.totalItems) * 100
                        : 0
                    }%`,
                  }}
                  title={`Rejected: ${simulationResult.rejectedItems}`}
                ></div>
                <div
                  className="bg-red-500 h-full transition-all duration-500"
                  style={{
                    width: `${
                      simulationResult.totalItems > 0
                        ? (simulationResult.failedItems / simulationResult.totalItems) * 100
                        : 0
                    }%`,
                  }}
                  title={`Failed: ${simulationResult.failedItems}`}
                ></div>
              </div>
              <div className="flex justify-between text-[11px] text-gray-400 mt-1">
                <span className="text-green-400">● {simulationResult.successfulItems} Sorted</span>
                <span className="text-amber-400">● {simulationResult.rejectedItems} Rejected</span>
                <span className="text-red-400">● {simulationResult.failedItems} Failed</span>
              </div>
            </div>

            <div>
              <span className="text-xs font-bold text-gray-400 uppercase block mb-2">
                System Health Monitoring Evaluation
              </span>
              <div className="flex items-center space-x-4 text-xs font-mono">
                <div className="bg-emerald-950/60 border border-emerald-800 text-emerald-300 px-3 py-1.5 rounded flex-1 text-center">
                  <span className="block text-[10px] text-emerald-400 font-sans">HEALTHY</span>
                  <span className="font-bold text-sm">{simulationResult.healthyChecks}</span>
                </div>
                <div className="bg-amber-950/60 border border-amber-800 text-amber-300 px-3 py-1.5 rounded flex-1 text-center">
                  <span className="block text-[10px] text-amber-400 font-sans">WARNINGS</span>
                  <span className="font-bold text-sm">{simulationResult.warningChecks}</span>
                </div>
                <div className="bg-red-950/60 border border-red-800 text-red-300 px-3 py-1.5 rounded flex-1 text-center">
                  <span className="block text-[10px] text-red-400 font-sans">CRITICAL</span>
                  <span className="font-bold text-sm">{simulationResult.criticalChecks}</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
