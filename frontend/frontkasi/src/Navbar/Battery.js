import React from 'react';
import './Battery.css';

function Battery({ voltage, percentage }) {
  return (
    <div className="battery-container m-2">
      <img src="/battery_outline.svg" alt="Battery Outline" className="battery-outline" />
      <div
        className="battery-fill"
        style={{ width: `${percentage}%` }}
      ></div>
      <div className="battery-text">
        {percentage}%
      </div>
      <div>
        { voltage } V
      </div>
    </div>
  );
}

export default Battery;
