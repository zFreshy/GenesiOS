import { useState } from "react";
import "./App.css";

function App() {
  return (
    <div className="desktop-container">
      {/* Desktop Background Area */}
      <div className="desktop-background">
        <div className="icon">
          <span className="icon-img">🌍</span>
          <span className="icon-label">Navegador</span>
        </div>
        <div className="icon">
          <span className="icon-img">📦</span>
          <span className="icon-label">Package Manager</span>
        </div>
        <div className="icon">
          <span className="icon-img">💻</span>
          <span className="icon-label">Editor (VSCode)</span>
        </div>
      </div>

      {/* Taskbar / Dock */}
      <div className="taskbar">
        <button className="start-btn">G</button>
        <div className="taskbar-icons">
          <button className="task-icon">🌍</button>
          <button className="task-icon">💻</button>
          <button className="task-icon active">Terminal</button>
        </div>
        <div className="taskbar-time">22:44</div>
      </div>
    </div>
  );
}

export default App;
