#pragma once

inline const char* scripts_js = R"rawliteral(

let socket = null;
let pendingEchoLines = 0;
let reconnectInterval = 1000; // ms
let responseTimeout = null;
let responseTimeoutDelay = 6000; // ms

function connectSocket() {
  socket = new WebSocket("ws://" + window.location.host + "/ws");

  socket.onopen = function () {
    hideWsLostPopup(); // Supprime le popup 
    console.log("[WebSocket] Connected");
  };

  socket.onmessage = function (event) {

    const output = document.getElementById("output");
    const lines = event.data.split("\n");

    // Clear response timeout
    clearTimeout(responseTimeout);
    hideWsLostPopup(); 

    if (pendingEchoLines > 0) {
      pendingEchoLines -= 1;
      return;
    }

    output.value += lines.join("\n");
    output.scrollTop = output.scrollHeight;
    console.log("[WebSocket] Recv:", event.data);
  };

  socket.onerror = function (error) {
    console.error("[WebSocket] Error:", error);
  };

  socket.onclose = function () {
    console.warn("[WebSocket] Disconnected. Retrying in 1s...");
    showWsLostPopup(); // Affiche le popup
    setTimeout(connectSocket, reconnectInterval);
  };
}

function showWsLostPopup() {
  const popup = document.getElementById("ws-lost-popup");
  if (popup) {
    popup.style.display = "block";
  }
}

function hideWsLostPopup() {
  const popup = document.getElementById("ws-lost-popup");
  if (popup) {
    popup.style.display = "none";
  }
}

function sendCommand() {
  const input = document.getElementById("command");
  const output = document.getElementById("output");
  const cmd = input.value.trim();

  // Timeout for response
  clearTimeout(responseTimeout);
  responseTimeout = setTimeout(() => {
    console.warn("[WebSocket] No response after command.");
    showWsLostPopup();
  }, responseTimeoutDelay);

  if (socket.readyState !== WebSocket.OPEN) return;

  socket.send(cmd + "\n");
  pendingEchoLines = cmd.length;

  input.value = "";

  // Don't save the cmd if  it just a number
  if (!/^\d+$/.test(cmd)) {
    output.value += cmd;
    addToHistory(cmd);
  }
}

// Bind Enter to sendCommand
window.addEventListener("DOMContentLoaded", function () {
  const input = document.getElementById("command");

  input.addEventListener("keydown", function (event) {
    if (event.key === "Enter") {
      event.preventDefault();
      sendCommand();
    }
  });

  // Initialiser l'affichage du terminal
  output.value = 
`  ____                    _           _       
 | __ ) _   _ ___   _ __ (_)_ __ __ _| |_ ___ 
 |  _ \\| | | / __| | '_ \\| | '__/ _\` | __/ _ \\
 | |_) | |_| \\__ \\ | |_) | | | | (_| | ||  __/
 |____/ \\__,_|___/ | .__/|_|_|  \\__,_|\\__\\___|
                   |_|                        
     Version 0.1           Ready to board

 Type 'mode' to start or 'help' for commands

HIZ> `;

  connectSocket(); // Initial connection
});

function addToHistory(cmd) {
  if (!isValidCommand(cmd)) return;
  const history = document.getElementById("history");

  const last = history.firstChild;
  if (last && last.textContent === cmd) return;

  const btn = document.createElement("button");
  
  // Limiter l'affichage du texte à 15 caractères
  const maxLength = 15;
  const displayText = cmd.length > maxLength ? cmd.slice(0, maxLength - 3) + "..." : cmd;

  btn.textContent = displayText;
  btn.title = cmd; // Tooltip complet au survol
  btn.onclick = () => {
    document.getElementById("command").value = cmd;
    document.getElementById("command").focus();
  };

  history.insertBefore(btn, history.firstChild);
}

function isValidCommand(cmd) {
  if (!cmd) return false;
  if (cmd.length < 2) return false;
  if (/^\d+$/.test(cmd)) return false;
  return true;
}

)rawliteral";
