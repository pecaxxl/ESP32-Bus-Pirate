#pragma once

inline const char* style_css = R"rawliteral(
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
  overflow-wrap: break-word;
}

html, body {
  height: 100%;
  width: 100%;
  max-width: 100vw;
  max-height: 100dvh;
  font-family: Menlo, 'Courier New', Courier, 'Liberation Mono', monospace;
  background-color: #121212;
  color: #e0e0e0;
  overflow: hidden;
  touch-action: manipulation;
}

main {
  display: flex;
  flex-direction: column;
  flex-grow: 1;
  height: 100dvh;
  padding: max(10px, env(safe-area-inset-top)) max(10px, env(safe-area-inset-right)) max(10px, env(safe-area-inset-bottom)) max(10px, env(safe-area-inset-left));
  gap: 10px;
  overflow: hidden;
  min-width: 0;
  max-width: 100%;
}

/* Terminal output */
#output {
  font-family: Menlo, 'Courier New', Courier, 'Liberation Mono', monospace;
  font-size: 0.9rem;
  letter-spacing: 0;
  line-height: 1.2;
  flex-grow: 1;
  min-height: 0;
  width: 100%;
  background: linear-gradient(145deg, #1a1a1a, #0f0f0f);
  color: #00ff00;
  border: 1px solid #333;
  border-radius: 8px;
  padding: 12px;
  font-size: 0.9rem;
  resize: none;
  box-shadow: inset 0 0 5px #000;
  box-sizing: border-box;
  overflow-y: auto;
  overflow-x: hidden;
  white-space: pre-wrap;
  word-break: break-word;
  caret-color: transparent;
}

#output:focus {
  outline: none;
  border-color: #00ffcc;
  box-shadow: 0 0 5px #00ffcc44;
}

#output::-webkit-scrollbar {
  display: none;
}

/* Input + Send Button */
.input-area {
  display: flex;
  gap: 8px;
  flex-shrink: 0;
  width: 100%;
  max-width: 100%;
  box-sizing: border-box;
  min-width: 0;
}

input[type="text"] {
  flex: 1;
  padding: 10px;
  background-color: #1c1c1c;
  color: #00ff00;
  border: 1px solid #333;
  border-radius: 8px;
  font-size: 1rem;
  outline: none;
  box-sizing: border-box;
  min-width: 0;
}
input[type="text"]:focus {
  border-color: #00ffcc;
  box-shadow: 0 0 5px #00ffcc44;
}

button {
  padding: 10px 16px;
  background: linear-gradient(145deg, #2a2a2a, #1a1a1a);
  color: #00ff00;
  border: 1px solid #444;
  border-radius: 8px;
  cursor: pointer;
  font-size: 1rem;
  white-space: nowrap;
  transition: background 0.2s ease, box-shadow 0.2s;
}
button:hover {
  background: #333;
  box-shadow: 0 0 6px #00ff0044;
}

/* History */
#history-title {
  margin: 4px 0 0 0;
  font-size: 1rem;
  display: none;
  flex-shrink: 0;
}

.history-area {
  display: flex;
  flex-wrap: nowrap;
  gap: 6px;
  white-space: nowrap;
  padding-bottom: 4px;
  flex-shrink: 0;
  width: 100%;
  max-width: 100%;
  overflow-x: auto;
  overflow-y: hidden;
  box-sizing: border-box;
  min-width: 0;
  -webkit-overflow-scrolling: touch;
}
.history-area::-webkit-scrollbar {
  width: 0;
  height: 0;
}
.history-area button:hover {
  background: #2a2a2a;
}
.history-area::-webkit-scrollbar {
  height: 6px;
}
.history-area::-webkit-scrollbar-thumb {
  background-color: #444;
  border-radius: 4px;
}
.history-area::-webkit-scrollbar-track {
  background-color: #222;
}

.history-area button {
  flex: 0 0 auto;
  background: #1a1a1a;
  color: #00ff00;
  border: 1px solid #333;
  border-radius: 6px;
  padding: 6px 10px;
  font-size: 0.85em;
  transition: background 0.2s ease;
}

.popup {
  position: fixed;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  background: #222;
  color: #ff5555;
  padding: 12px 18px;
  border: 1px solid #444;
  border-radius: 8px;
  box-shadow: 0 0 8px #000;
  z-index: 9999;
  font-size: 0.95rem;
  max-width: 90vw;

  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;

  white-space: nowrap;
  overflow: hidden;
}

.popup-text {
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.popup a {
  color: #00ffcc;
  text-decoration: underline;
  flex-shrink: 0;
}

.popup a:hover {
  color: #00ffff;
}
  

/* Responsive */
@media (max-width: 600px), (max-height: 600px) {
  main {
    padding: 8px;
    gap: 6px;
  }

  .popup {
    font-size: 0.8rem;
  }


  #output {
    font-size: 0.8rem;
    padding: 8px;
  }

  .input-area {
    flex-direction: column;
    gap: 6px;
  }

  input[type="text"] {
    font-size: 0.95rem;
    width: 100%;
  }

  .input-area button {
    width: 100%;
    font-size: 0.95rem;
  }

  .history-area {
    gap: 4px;
  }

  .history-area button {
    font-size: 0.75em;
    padding: 5px 8px;
  }
}

)rawliteral";
