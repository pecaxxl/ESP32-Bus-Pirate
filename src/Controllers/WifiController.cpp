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
    } else if (root == "spoof") {
        handleSpoof(cmd);
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
        terminalView.println("");
        terminalView.println("WiFi: Connected! WebUI IP " + wifiService.getLocalIP());
        terminalView.println("      Reset the device and choose WiFi Web,");
        terminalView.println("      if you want to use the web based CLI");
        terminalView.println("");

        // Save creds
        nvsService.open();
        nvsService.saveString(state.getNvsSsidField(), ssid);
        nvsService.saveString(state.getNvsPasswordField(), password);
        nvsService.close();

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


/*
Status
*/
void WifiController::handleStatus(const TerminalCommand& cmd) {
    if (wifiService.isConnected()) {
        terminalView.println("WiFi: Connected");
        terminalView.println("  IP:  " + wifiService.getLocalIP());
    } else {
        terminalView.println("WiFi: Not connected.");
    }
    
    terminalView.println("  STA MAC: " + wifiService.getMacAddressSta());
    terminalView.println("   AP MAC: " + wifiService.getMacAddressAp());
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
    delay(300);

    auto networks = wifiService.scanDetailedNetworks();

    for (const auto& net : networks) {
        std::string line = "  SSID: " + net.ssid;
        line += " | Sec: " + wifiService.encryptionTypeToString(net.encryption);
        line += " | BSSID: " + net.bssid;
        line += " | CH: " + std::to_string(net.channel);
        line += " | RSSI: " + std::to_string(net.rssi) + " dBm";
        if (net.open) line += " [open]";
        if (net.vulnerable) line += " [vulnerable]";
        if (net.hidden) line += " [hidden]";

        terminalView.println(line);
    }


    if (networks.empty()) {
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
    terminalView.println("WiFi Sniffing started... Press [Enter] to stop.\n");

    wifiService.startPassiveSniffing();
    wifiService.switchChannel(1);

    uint8_t channel = 1;
    unsigned long lastHop = 0;
    unsigned long lastPull = 0;

    while (true) {
        // Enter Press
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        // Read sniff data
        if (millis() - lastPull > 20) {
            auto logs = wifiService.getSniffLog();
            for (const auto& line : logs) {
                terminalView.println(line);
            }
            lastPull = millis();
        }

        // Switch channel every 100ms
        if (millis() - lastHop > 100) {
            channel = (channel % 13) + 1;  // channel 1 to 13
            wifiService.switchChannel(channel);
            lastHop = millis();
        }

        delay(5);
    }

    wifiService.stopPassiveSniffing();
    terminalView.println("WiFi Sniffing stopped.\n");
}

/*
Spoof
*/
void WifiController::handleSpoof(const TerminalCommand& cmd) {
    auto mode = cmd.getSubcommand();
    auto mac= cmd.getArgs();

    if (mode.empty() && mac.empty()) {
        terminalView.println("Usage: spoof sta <mac>");
        terminalView.println("       spoof ap <mac>");
        return;
    }

    WifiService::MacInterface iface = (mode == "sta") 
        ? WifiService::MacInterface::Station
        : WifiService::MacInterface::AccessPoint;

    terminalView.println("WiFi: Spoofing " + mode + " MAC to " + mac + "...");

    bool ok = wifiService.spoofMacAddress(mac, iface);

    if (ok) {
        terminalView.println("WiFi: MAC spoofed successfully.");
    } else {
        terminalView.println("WiFi: Failed to spoof MAC.");
    }
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
        auto ip = wifiService.getLocalIP();
        terminalView.println("");
        terminalView.println("[WARNING] If you're connected via serial,");
        terminalView.println("          the web UI will not be active.");
        terminalView.println("          Reset the device and choose WiFi Web.");
        terminalView.println("");
        terminalView.println("WiFi Web UI: http://" + ip);
    } else {
        terminalView.println("WiFi Web UI: Not connected. Connect first to see address.");
    }
}

/*
Config
*/
void WifiController::handleConfig() {
    terminalView.println("[WARNING] If you're connected via Web CLI,");
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
    terminalView.println("  spoof sta <mac>");
    terminalView.println("  spoof ap <mac>");
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