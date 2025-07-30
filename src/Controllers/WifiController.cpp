#include "Controllers/WifiController.h"

/*
Constructor
*/
WifiController::WifiController(ITerminalView& terminalView, IInput& terminalInput, IInput& deviceInput, WifiService& wifiService, SshService& sshService, NvsService& nvsService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), deviceInput(deviceInput), wifiService(wifiService), sshService(sshService), nvsService(nvsService), argTransformer(argTransformer) {}

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
    } else if (root == "ssh") {
        handleSsh(cmd);
    } else if (root == "reset") {
        handleReset();
    } else if (root == "deauth") {
        handleDeauth(cmd);
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
        terminalView.println("WiFi: Connected to Wi-Fi!");
        terminalView.println("      Reset the device and choose WiFi Web,");
        terminalView.println("      if you want to use the web based CLI");
        terminalView.println("");
        terminalView.println("[BAREBONE] To launch the WebUI without a screen:");
        terminalView.println("  1. Reset the device (don’t hold the board button during boot)");
        terminalView.println("  1. Once the device is powered, you have 3 seconds to press the board button");
        terminalView.println("  3. The built-in LED shows the following status:");
        terminalView.println("     • Blue  = No Wi-Fi credentials saved.");
        terminalView.println("     • White = Connecting in progress");
        terminalView.println("     • Green = Connected, open the WebUI in a browser");
        terminalView.println("     • Red   = Connection failed, try connect again with serial");
        terminalView.println("");
        terminalView.println("WiFi Web UI: http://" + wifiService.getLocalIP());

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
        terminalView.println("WiFi: Access Point started with SSID " + ssid);
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

    int responseTimeMs = wifiService.ping(host);

    if (responseTimeMs >= 0) {
        terminalView.println("WiFi: Ping on " + host + " successful (" + std::to_string(responseTimeMs) + " ms).");
    } else {
        terminalView.println("WiFi: Ping failed.");
    }
}

/*
Sniff
*/
void WifiController::handleSniff(const TerminalCommand& cmd) {
    terminalView.println("WiFi Sniffing started... Press [ENTER] to stop.\n");

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
        terminalView.println("[BAREBONE] To launch the WebUI without a screen:");
        terminalView.println("  1. Reset the device (don’t hold the board button during boot)");
        terminalView.println("  1. Once the device is powered, you have 3 seconds to press the board button");
        terminalView.println("  3. The built-in LED shows the following status:");
        terminalView.println("     • Blue  = No Wi-Fi credentials saved.");
        terminalView.println("     • White = Connecting in progress");
        terminalView.println("     • Green = Connected, open the WebUI in your browser.");
        terminalView.println("     • Red   = Connection failed, try connect again with serial");
        terminalView.println("");
        terminalView.println("WiFi Web UI: http://" + ip);
    } else {
        terminalView.println("WiFi Web UI: Not connected. Connect first to see address.");
    }
}

/*
SSH
*/
void WifiController::handleSsh(const TerminalCommand& cmd) {
    // Check connection
    if (!wifiService.isConnected()) {
        terminalView.println("SSH: You must be connected to Wi-Fi. Use 'connect' first.");
        return;
    }

    // Check args
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (cmd.getSubcommand().empty() || args.size() < 2) {
        terminalView.println("Usage: ssh <host> <user> <password> [port]");
        return;
    }

    // Check port
    int port = 22;
    if (args.size() == 3) {
        if (argTransformer.isValidNumber(args[2])) {
            port = argTransformer.parseHexOrDec16(args[2]);
        } 
    }

    std::string host = cmd.getSubcommand();
    std::string user = args[0];
    std::string pass = args[1];

    // Connect, start the ssh task
    terminalView.println("SSH: Connecting to " + host + " as " + user + " with port " + std::to_string(port) + "...");
    sshService.startTask(host, user, pass, false, port);

    // Wait 5sec for connection success
    unsigned long start = millis();
    while (!sshService.isConnected() && millis() - start < 5000) {
        delay(500);
    }

    // Can't connect
    if (!sshService.isConnected()) {
        terminalView.println("\r\nSSH: Connection failed.");
        sshService.close();
        return;
    }

    // Connected, start the bridge loop
    terminalView.println("SSH: Connected. Shell started... Press [ANY ESP32 KEY] to stop.\n");
    while (true) {
        char terminalKey = terminalInput.readChar();
        if (terminalKey != KEY_NONE) sshService.writeChar(terminalKey);

        char deviceKey = deviceInput.readChar();
        if (deviceKey != KEY_NONE) break;

        std::string output = sshService.readOutputNonBlocking();
        if (!output.empty()) terminalView.print(output);

        delay(10);
    }

    // Close SSH
    sshService.close();
    terminalView.println("\r\n\nSSH: Session closed.");
}

/*
Config
*/
void WifiController::handleConfig() {
    terminalView.println("[WARNING] If you're connected via Web CLI,");
    terminalView.println("          executing Wi-Fi commands may cause ");
    terminalView.println("          the terminal session to disconnect.");
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
    terminalView.println("  ssh <host> <username> <password> [port]");
    terminalView.println("  webui");
    terminalView.println("  reset");
    terminalView.println("  deauth <ap> [bursts]");
}

/*
Ensure Configuration
*/
void WifiController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}

/*
Deathenticate stations attack
*/
void WifiController::handleDeauth(const TerminalCommand& cmd)
{
    auto target = cmd.getSubcommand();

    // if the SSID have space in name, e.g "Router Wifi"
    if (!cmd.getArgs().empty()) {
        target =+ " " + cmd.getArgs();
    }
    
    if (target.empty()) {
        terminalView.println("Usage: deauth <ssid>");
        return;
    }

    terminalView.println("WiFi: Sending deauth to \"" + target + "\"...");

    bool ok = wifiService.deauthApBySsid(target);

    if (ok) terminalView.println("WiFi: Deauth frames sent.");
    else    terminalView.println("WiFi: SSID not found.");
}
