/**
 * WasteItemForm Component
 * Live Waste Sorting Console Input Form.
 * Allows operator to submit item parameters for real-time classification & diverter gate actuation.
 */
import React, { useState } from 'react';
import { IconDice, IconZap, IconAlertTriangle } from './Icons';

const SUPPORTED_TYPES = ['PLASTIC', 'METAL', 'PAPER', 'GLASS', 'OTHER'];

export function WasteItemForm({ onSubmit, isSubmitting, isConnected }) {
  const [formData, setFormData] = useState({
    id: 'UI-PLASTIC-001',
    type: 'PLASTIC',
    confidence: '0.95',
    gateId: '1',
    bboxX: '10',
    bboxY: '10',
    bboxWidth: '100',
    bboxHeight: '80',
  });

  const [validationError, setValidationError] = useState('');

  const handleChange = (e) => {
    const { name, value } = e.target;
    setFormData((prev) => ({ ...prev, [name]: value }));
  };

  const handleGenerateRandomId = () => {
    const types = ['PLASTIC', 'METAL', 'PAPER', 'GLASS'];
    const randomType = types[Math.floor(Math.random() * types.length)];
    const randomNum = Math.floor(100 + Math.random() * 900);
    setFormData((prev) => ({
      ...prev,
      id: `UI-${randomType}-${randomNum}`,
      type: randomType,
    }));
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    setValidationError('');

    if (!formData.id || !formData.id.trim()) {
      setValidationError('Validation Error: Waste Item ID cannot be empty.');
      return;
    }

    if (!SUPPORTED_TYPES.includes(formData.type)) {
      setValidationError(`Validation Error: Unsupported waste type '${formData.type}'.`);
      return;
    }

    const confVal = parseFloat(formData.confidence);
    if (isNaN(confVal) || confVal < 0.0 || confVal > 1.0) {
      setValidationError('Validation Error: Confidence must be a numeric value between 0.00 and 1.00.');
      return;
    }

    const gateVal = parseInt(formData.gateId, 10);
    if (isNaN(gateVal) || gateVal < 0) {
      setValidationError('Validation Error: Gate ID must be a non-negative integer.');
      return;
    }

    const bboxX = parseInt(formData.bboxX, 10);
    const bboxY = parseInt(formData.bboxY, 10);
    const bboxWidth = parseInt(formData.bboxWidth, 10);
    const bboxHeight = parseInt(formData.bboxHeight, 10);

    if (isNaN(bboxX) || isNaN(bboxY) || isNaN(bboxWidth) || isNaN(bboxHeight) || bboxWidth <= 0 || bboxHeight <= 0) {
      setValidationError('Validation Error: Bounding Box X/Y must be non-negative and Width/Height must be > 0.');
      return;
    }

    const payload = {
      id: formData.id.trim(),
      timestampMs: Date.now(),
      type: formData.type,
      confidence: confVal,
      gateId: gateVal,
      boundingBox: {
        x: bboxX,
        y: bboxY,
        width: bboxWidth,
        height: bboxHeight,
      },
    };

    onSubmit(payload);
  };

  return (
    <form className="waste-item-form" onSubmit={handleSubmit} id="waste-item-form">
      <div className="form-header">
        <h4 className="form-section-title">
          📥 Waste Item Payload Parameters
        </h4>
        <button
          type="button"
          className="btn-industrial btn-sm btn-reset"
          onClick={handleGenerateRandomId}
          disabled={isSubmitting || !isConnected}
          title="Generate sample item ID & type"
          id="btn-random-id"
        >
          <IconDice size={14} /> Random Payload
        </button>
      </div>

      {validationError && (
        <div className="validation-error-box" id="validation-error-box">
          <IconAlertTriangle size={16} />
          <span>{validationError}</span>
        </div>
      )}

      <div className="form-grid">
        {/* Waste ID */}
        <div className="form-group">
          <label className="form-label" htmlFor="input-item-id">Waste Item ID</label>
          <input
            type="text"
            id="input-item-id"
            name="id"
            className="form-input font-mono"
            value={formData.id}
            onChange={handleChange}
            disabled={isSubmitting || !isConnected}
            placeholder="e.g. UI-PLASTIC-001"
            required
          />
        </div>

        {/* Waste Type Dropdown */}
        <div className="form-group">
          <label className="form-label" htmlFor="select-waste-type">Waste Classification Type</label>
          <select
            id="select-waste-type"
            name="type"
            className="form-select font-mono"
            value={formData.type}
            onChange={handleChange}
            disabled={isSubmitting || !isConnected}
          >
            {SUPPORTED_TYPES.map((t) => (
              <option key={t} value={t}>
                {t}
              </option>
            ))}
          </select>
        </div>

        {/* Confidence Input */}
        <div className="form-group">
          <label className="form-label" htmlFor="input-confidence">
            Confidence Score (0.00 – 1.00)
          </label>
          <input
            type="number"
            id="input-confidence"
            name="confidence"
            className="form-input font-mono"
            step="0.01"
            min="0.00"
            max="1.00"
            value={formData.confidence}
            onChange={handleChange}
            disabled={isSubmitting || !isConnected}
            required
          />
        </div>

        {/* Gate ID Input */}
        <div className="form-group">
          <label className="form-label" htmlFor="input-gate-id">Diverter Gate Target ID</label>
          <input
            type="number"
            id="input-gate-id"
            name="gateId"
            className="form-input font-mono"
            min="0"
            value={formData.gateId}
            onChange={handleChange}
            disabled={isSubmitting || !isConnected}
            required
          />
        </div>
      </div>

      {/* Bounding Box Inputs Group */}
      <div className="form-sub-section">
        <label className="form-label">Bounding Box Coordinates (Pixels)</label>
        <div className="bbox-grid">
          <div className="form-group">
            <span className="input-prefix">X Pos</span>
            <input
              type="number"
              name="bboxX"
              className="form-input font-mono"
              value={formData.bboxX}
              onChange={handleChange}
              disabled={isSubmitting || !isConnected}
            />
          </div>
          <div className="form-group">
            <span className="input-prefix">Y Pos</span>
            <input
              type="number"
              name="bboxY"
              className="form-input font-mono"
              value={formData.bboxY}
              onChange={handleChange}
              disabled={isSubmitting || !isConnected}
            />
          </div>
          <div className="form-group">
            <span className="input-prefix">Width</span>
            <input
              type="number"
              name="bboxWidth"
              className="form-input font-mono"
              value={formData.bboxWidth}
              onChange={handleChange}
              disabled={isSubmitting || !isConnected}
            />
          </div>
          <div className="form-group">
            <span className="input-prefix">Height</span>
            <input
              type="number"
              name="bboxHeight"
              className="form-input font-mono"
              value={formData.bboxHeight}
              onChange={handleChange}
              disabled={isSubmitting || !isConnected}
            />
          </div>
        </div>
      </div>

      {/* Submit Button */}
      <button
        type="submit"
        className="btn-industrial btn-submit-sorting"
        disabled={isSubmitting || !isConnected}
        id="btn-process-waste"
      >
        <IconZap size={18} />
        {isSubmitting ? 'PROCESSING ITEM...' : 'PROCESS WASTE ITEM'}
      </button>
    </form>
  );
}

export default WasteItemForm;
