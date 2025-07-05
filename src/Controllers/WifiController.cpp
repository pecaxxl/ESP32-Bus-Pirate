#include "Controllers/WifiController.h"

/*
Constructor
*/
WifiController::WifiController(ITerminalView& terminalView, IInput& terminalInput, WifiService& wifiService, NvsService& nvsService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), wifiService(wifiService), nvsService(nvsService), argTransformer(argTransformer) {}

/*
Entry point for command
*/
void WifiController::handleCommand(const TerminalCommand& cmd) {
    const auto& root = cmd.getRoot();

    if (root == "connect") {
        handleConnect(cmd);
    } else if (root == "disconnect") {
        handleDisconnect(cmd);
    } else if (root == "status") {
        handleStatus(cmd);
    } else if (root == "ap") {
        handleAp(cmd);
    } else if (root == "scan") {
        handleScan(cmd);
    } else if (root == "ping") {
        handlePing(cmd);
    } else if (root == "sniff") {
        handleSniff(cmd);
    } else if (root == "webui") {
        handleWebUi(cmd);
    } else if (root == "reset") {
        handleReset();
    } else {
        handleHelp();
    }
}

/*
Connect
*/
void WifiController::handleConnect(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getSubcommand());

    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: connect <ssid> <password>");
        return;
    }

    std::string ssid = cmd.getSubcommand();
    std::string password = cmd.getArgs();

    terminalView.println("WiFi: Connecting to " + ssid + "...");

    wifiService.setModeApSta();
    wifiService.connect(ssid, password);
    if (wifiService.isConnected()) {
        terminalView.println("WiFi: Connected! IP " + wifiService.getLocalIP());
    } else {
        terminalView.println("WiFi: Connection failed.");
    }
}

/*
Disconnect
*/
void WifiController::handleDisconnect(const TerminalCommand& cmd) {
    wifiService.disconnect();
    terminalView.println("WiFi: Disconnected.");
}

void WifiController::handleStatus(const TerminalCommand& cmd) {
    if (wifiService.isConnected()) {
        terminalView.println("WiFi: Connected");
        terminalView.println("  IP:  " + wifiService.getLocalIP());
        terminalView.println("  MAC: " + wifiService.getMacAddress());
    } else {
        terminalView.println("WiFi: Not connected.");
    }
}

/*
Access Point
*/
void WifiController::handleAp(const TerminalCommand& cmd) {
    auto ssid = cmd.getSubcommand();

    if (ssid.empty()) {
        terminalView.println("Usage: ap <ssid> <password>");
        return;
    }

    std::string password = cmd.getArgs();

    // Already connected, mode AP+STA
    if (wifiService.isConnected()) {
        wifiService.setModeApSta();
    } else {
        wifiService.setModeApOnly();
    }

    if (wifiService.startAccessPoint(ssid, password)) {
        terminalView.println("WiFi: Access Point started with SSID = " + ssid);
        terminalView.println("AP IP: " + wifiService.getApIp());

        auto nvsSsidField = state.getNvsSsidField();
        auto nvsPasswordField = state.getNvsPasswordField();
        auto ssid = nvsService.getString(nvsSsidField, "");
        auto password = nvsService.getString(nvsPasswordField, "");

        // Try to reconnect to saved WiFi
        if (!ssid.empty() && !password.empty()) {
            wifiService.connect(ssid, password);
        }

        if (wifiService.isConnected()) {
            terminalView.println("STA IP: " + wifiService.getLocalIp());
        }
    } else {
        terminalView.println("WiFi: Failed to start Access Point.");
    }
}

/*
Scan
*/
void WifiController::handleScan(const TerminalCommand&) {
    terminalView.println("WiFi: Scanning for networks...");

    auto results = wifiService.scanNetworks();
    for (const auto& network : results) {
        terminalView.println("  SSID: " + network);
    }

    if (results.empty()) {
        terminalView.println("WiFi: No networks found.");
    }
}

/*
Ping
*/
void WifiController::handlePing(const TerminalCommand& cmd) {
    std::string host = cmd.getSubcommand();

    if (host.empty()) {
        terminalView.println("Usage: ping <host>");
        return;
    }

    if (!wifiService.isConnected()) {
        terminalView.println("WiFi: Not connected.");
        return;
    }

    terminalView.println("WiFi: Pinging " + host + "...");

    if (wifiService.ping(host)) {
        terminalView.println("WiFi: Ping successful.");
    } else {
        terminalView.println("WiFi: Ping failed.");
    }
}

/*
Sniff
*/
void WifiController::handleSniff(const TerminalCommand& cmd) {
    terminalView.println("WiFi Sniff: Not implemented yet.");
}

/*
Reset
*/
void WifiController::handleReset() {
    wifiService.reset();
    terminalView.println("WiFi: Interface reset. Disconnected.");
}

/*
Web Interface
*/
void WifiController::handleWebUi(const TerminalCommand&) {
    if (wifiService.isConnected()) {
        auto ip = wifiService.getCurrentIP();
        terminalView.println("WiFi: http://" + ip);
    } else {
        terminalView.println("WiFi: Not connected. Connect first to see Web UI address.");
    }
}

/*
Config
*/
void WifiController::handleConfig() {
    terminalView.println("[WARNING] If you're connected via Wi-Fi,");
    terminalView.println("          executing Wi-Fi configuration commands may");
    terminalView.println("          cause the terminal session to disconnect.");
    terminalView.println("          Use USB serial or restart if connection is lost.\n");
}

/*
Help
*/
void WifiController::handleHelp() {
    terminalView.println("WiFi commands:");
    terminalView.println("  scan");
    terminalView.println("  ping <host>");
    terminalView.println("  sniff");
    terminalView.println("  connect <ssid> <password>");
    terminalView.println("  status");
    terminalView.println("  disconnect");
    terminalView.println("  ap <ssid> <password>");
    terminalView.println("  webui");
    terminalView.println("  reset");
}

void WifiController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}