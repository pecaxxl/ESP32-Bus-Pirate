#pragma once

inline const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Bus Pirate</title>
  <link rel="stylesheet" href="/style.css">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover, user-scalable=no">
</head>
<body>
  <main>
    <textarea id="output" readonly></textarea>
    <div class="input-area">
      <input type="text" id="command" placeholder="Enter command" autocapitalize="off" autocomplete="off" autocorrect="off" spellcheck="false">
      <button onclick="sendCommand()">Send</button>
    </div>
    <h3 id="history-title" style="display: none;">Command History</h3>
    <div id="history" class="history-area"></div>
  </main>
  <div id="ws-lost-popup" class="popup" style="display: none;">
    Connection lost. <a href="#" onclick="location.reload()">Refresh</a>
  </div>
  <script src="/scripts.js"></script>
</body>
</html>
)rawliteral";
