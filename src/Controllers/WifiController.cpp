#include "Controllers/WifiController.h"

/*
Constructor
*/
WifiController::WifiController(
    ITerminalView &terminalView,
    IInput &terminalInput,
    IInput &deviceInput,
    WifiService &wifiService,
    WifiOpenScannerService &wifiScannerService,
    SshService &sshService,
    NetcatService &netcatService,
    NmapService &nmapService,
    NvsService &nvsService,
    ArgTransformer &argTransformer,
    UserInputManager &userInputManager
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceInput(deviceInput),
      wifiService(wifiService),
      wifiScannerService(wifiScannerService),
      sshService(sshService),
      netcatService(netcatService),
      nmapService(nmapService),
      nvsService(nvsService),
      argTransformer(argTransformer),
      userInputManager(userInputManager)
{}

/*
Entry point for command
*/
void WifiController::handleCommand(const TerminalCommand &cmd)
{
    const auto &root = cmd.getRoot();

    if (root == "connect") handleConnect(cmd);
    else if (root == "disconnect") handleDisconnect(cmd);
    else if (root == "status") handleStatus(cmd);
    else if (root == "ap") handleAp(cmd);
    else if (root == "spoof") handleSpoof(cmd);
    else if (root == "scan") handleScan(cmd);
    else if (root == "probe") handleProbe();
    else if (root == "ping") handlePing(cmd);
    else if (root == "sniff") handleSniff(cmd);
    else if (root == "webui") handleWebUi(cmd);
    else if (root == "ssh") handleSsh(cmd);
    else if (root == "nc") handleNetcat(cmd);
    else if (root == "nmap") handleNmap(cmd);
    else if (root == "reset") handleReset();
    else if (root == "deauth") handleDeauth(cmd);
    else handleHelp();
}

/*
Connect
*/
void WifiController::handleConnect(const TerminalCommand &cmd)
{
    std::string ssid;
    std::string password;
    auto args = argTransformer.splitArgs(cmd.getSubcommand());

    // No args provided, we need to scan and select networks
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Wifi: Scanning for available networks...");
        auto networks = wifiService.scanNetworks();
        int selectedIndex = userInputManager.readValidatedChoiceIndex("\nSelect Wi-Fi network", networks, 0);
        ssid = networks[selectedIndex];
        terminalView.println("Selected SSID: " + ssid);
        terminalView.print("Password: ");
        password = userInputManager.getLine();

    // Args provided
    } else  {
        // Concatenate subcommand and args
        std::string full = cmd.getSubcommand() + " " + cmd.getArgs();
    
        // Find the last space to separate SSID and password
        size_t pos = full.find_last_of(' ');
        if (pos == std::string::npos || pos == full.size() - 1) {
            terminalView.println("Usage: connect <ssid> <password>");
            return;
        }
        ssid = full.substr(0, pos);
        password = full.substr(pos + 1);
    }

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
        wifiService.reset();
        delay(100);
    }
}

/*
Disconnect
*/
void WifiController::handleDisconnect(const TerminalCommand &cmd)
{
    wifiService.disconnect();
    terminalView.println("WiFi: Disconnected.");
}

/*
Status
*/
void WifiController::handleStatus(const TerminalCommand &cmd)
{
    auto ssid     = wifiService.getSsid();     if (ssid.empty()) ssid = "N/A";
    auto bssid    = wifiService.getBssid();    if (bssid.empty()) bssid = "N/A";
    auto hostname = wifiService.getHostname(); if (hostname.empty()) hostname = "N/A";

    terminalView.println("\n=== Wi-Fi Status ===");
    terminalView.println("Mode         : " + std::string(wifiService.getWifiModeRaw() == WIFI_MODE_AP ? "Access Point" : "Station"));
    terminalView.println("AP MAC       : " + wifiService.getMacAddressAp());
    terminalView.println("STA MAC      : " + wifiService.getMacAddressSta());
    terminalView.println("IP           : " + wifiService.getLocalIp());
    terminalView.println("Subnet       : " + wifiService.getSubnetMask());
    terminalView.println("Gateway      : " + wifiService.getGatewayIp());
    terminalView.println("DNS1         : " + wifiService.getDns1());
    terminalView.println("DNS2         : " + wifiService.getDns2());
    terminalView.println("Hostname     : " + hostname);

    terminalView.println("SSID         : " + ssid);
    terminalView.println("BSSID        : " + bssid);
    terminalView.println("Prov enabled : " + std::string(wifiService.isProvisioningEnabled() ? "Yes" : "No"));

    const int status = wifiService.getWifiStatusRaw();
    if (status == 3 /* WL_CONNECTED */) {
        terminalView.println("RSSI         : " + std::to_string(wifiService.getRssi()) + " dBm");
        terminalView.println("Channel      : " + std::to_string(wifiService.getChannel()));
    } else {
        terminalView.println("RSSI         : N/A");
        terminalView.println("Channel      : N/A");
    }

    terminalView.println("Mode         : " + std::string(wifiService.wifiModeToStr(wifiService.getWifiModeRaw())));
    terminalView.println("Status       : " + std::string(wifiService.wlStatusToStr(status)));
    terminalView.println("Prov enabled : " + std::string(wifiService.isProvisioningEnabled() ? "Yes" : "No"));
    terminalView.println("====================\n");
}

/*
Access Point
*/
void WifiController::handleAp(const TerminalCommand &cmd)
{
    auto ssid = cmd.getSubcommand();

    if (ssid.empty())
    {
        terminalView.println("Usage: ap <ssid> <password>");
        return;
    }

    std::string password = cmd.getArgs();

    // Already connected, mode AP+STA
    if (wifiService.isConnected())
    {
        wifiService.setModeApSta();
    }
    else
    {
        wifiService.setModeApOnly();
    }

    if (wifiService.startAccessPoint(ssid, password))
    {
        terminalView.println("WiFi: Access Point started with SSID " + ssid);
        terminalView.println("AP IP: " + wifiService.getApIp());

        auto nvsSsidField = state.getNvsSsidField();
        auto nvsPasswordField = state.getNvsPasswordField();
        auto ssid = nvsService.getString(nvsSsidField, "");
        auto password = nvsService.getString(nvsPasswordField, "");

        // Try to reconnect to saved WiFi
        if (!ssid.empty() && !password.empty())
        {
            wifiService.connect(ssid, password);
        }

        if (wifiService.isConnected())
        {
            terminalView.println("STA IP: " + wifiService.getLocalIp());
        }
    }
    else
    {
        terminalView.println("WiFi: Failed to start Access Point.");
    }
}

/*
Scan
*/
void WifiController::handleScan(const TerminalCommand &)
{
    terminalView.println("WiFi: Scanning for networks...");
    delay(300);

    auto networks = wifiService.scanDetailedNetworks();

    for (const auto &net : networks)
    {
        std::string line = "  SSID: " + net.ssid;
        line += " | Sec: " + wifiService.encryptionTypeToString(net.encryption);
        line += " | BSSID: " + net.bssid;
        line += " | CH: " + std::to_string(net.channel);
        line += " | RSSI: " + std::to_string(net.rssi) + " dBm";
        if (net.open)
            line += " [open]";
        if (net.vulnerable)
            line += " [vulnerable]";
        if (net.hidden)
            line += " [hidden]";

        terminalView.println(line);
    }

    if (networks.empty())
    {
        terminalView.println("WiFi: No networks found.");
    }
}

/*
Probe
*/
void WifiController::handleProbe() 
{
    terminalView.println("WIFI: Starting probe for internet access on open networks...");
    terminalView.println("\n[WARNING] This will try to connect to surrounding open networks.\n");

    // Confirm before starting
    auto confirmation = userInputManager.readYesNo("Start Wi-Fi probe to find internet access?", false);
    if (!confirmation) {
        terminalView.println("WIFI: Probe cancelled.\n");
        return;
    }

    // Stop any existing probe
    if (wifiScannerService.isOpenProbeRunning()) {
        wifiScannerService.stopOpenProbe();
    }
    wifiScannerService.clearProbeLog();

    // Start the open probe service
    if (!wifiScannerService.startOpenProbe()) {
        terminalView.println("WIFI: Failed to start probe.\n");
        return;
    }

    terminalView.println("WIFI: Probe for internet access... Press [ENTER] to stop.\n");

    // Start the open probe task
    while (wifiScannerService.isOpenProbeRunning()) {
        // Display logs
        auto batch = wifiScannerService.fetchProbeLog();
        for (auto& ln : batch) {
            terminalView.println(ln.c_str());
        }

        // Enter Press to stop
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            wifiScannerService.stopOpenProbe();
            break;
        }

        delay(10);
    }

    // Flush final logs
    for (auto& ln : wifiScannerService.fetchProbeLog()) {
        terminalView.println(ln.c_str());
    }
    terminalView.println("WIFI: Open-Wifi probe ended.\n");
}

/*
Ping
*/
void WifiController::handlePing(const TerminalCommand &cmd)
{
    std::string host = cmd.getSubcommand();

    int responseTimeMs = wifiService.ping(host);

    if (responseTimeMs >= 0)
    {
        terminalView.println("WiFi: Ping on " + host + " successful (" + std::to_string(responseTimeMs) + " ms).");
    }
    else
    {
        terminalView.println("WiFi: Ping failed.");
    }
}

/*
Sniff
*/
void WifiController::handleSniff(const TerminalCommand &cmd)
{
    terminalView.println("WiFi Sniffing started... Press [ENTER] to stop.\n");

    wifiService.startPassiveSniffing();
    wifiService.switchChannel(1);

    uint8_t channel = 1;
    unsigned long lastHop = 0;
    unsigned long lastPull = 0;

    while (true)
    {
        // Enter Press
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n')
            break;

        // Read sniff data
        if (millis() - lastPull > 20)
        {
            auto logs = wifiService.getSniffLog();
            for (const auto &line : logs)
            {
                terminalView.println(line);
            }
            lastPull = millis();
        }

        // Switch channel every 100ms
        if (millis() - lastHop > 100)
        {
            channel = (channel % 13) + 1; // channel 1 to 13
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
void WifiController::handleSpoof(const TerminalCommand &cmd)
{
    auto mode = cmd.getSubcommand();
    auto mac = cmd.getArgs();

    if (mode.empty() && mac.empty())
    {
        terminalView.println("Usage: spoof sta <mac>");
        terminalView.println("       spoof ap <mac>");
        return;
    }

    WifiService::MacInterface iface = (mode == "sta")
                                          ? WifiService::MacInterface::Station
                                          : WifiService::MacInterface::AccessPoint;

    terminalView.println("WiFi: Spoofing " + mode + " MAC to " + mac + "...");

    bool ok = wifiService.spoofMacAddress(mac, iface);

    if (ok)
    {
        terminalView.println("WiFi: MAC spoofed successfully.");
    }
    else
    {
        terminalView.println("WiFi: Failed to spoof MAC.");
    }
}

/*
Reset
*/
void WifiController::handleReset()
{
    wifiService.reset();
    terminalView.println("WiFi: Interface reset. Disconnected.");
}

/*
Web Interface
*/
void WifiController::handleWebUi(const TerminalCommand &)
{
    if (wifiService.isConnected())
    {
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
    }
    else
    {
        terminalView.println("WiFi Web UI: Not connected. Connect first to see address.");
    }
}

/*
SSH
*/
void WifiController::handleSsh(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected())
    {
        terminalView.println("SSH: You must be connected to Wi-Fi. Use 'connect' first.");
        return;
    }

    // Check args
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (cmd.getSubcommand().empty() || args.size() < 2)
    {
        terminalView.println("Usage: ssh <host> <user> <password> [port]");
        return;
    }

    // Check port
    int port = 22;
    if (args.size() == 3)
    {
        if (argTransformer.isValidNumber(args[2]))
        {
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
    while (!sshService.isConnected() && millis() - start < 5000)
    {
        delay(500);
    }

    // Can't connect
    if (!sshService.isConnected())
    {
        terminalView.println("\r\nSSH: Connection failed.");
        sshService.close();
        return;
    }

    // Connected, start the bridge loop
    terminalView.println("SSH: Connected. Shell started... Press [ANY ESP32 KEY] to stop.\n");
    while (true)
    {
        char terminalKey = terminalInput.readChar();
        if (terminalKey != KEY_NONE)
            sshService.writeChar(terminalKey);

        char deviceKey = deviceInput.readChar();
        if (deviceKey != KEY_NONE)
            break;

        std::string output = sshService.readOutputNonBlocking();
        if (!output.empty())
            terminalView.print(output);

        delay(10);
    }

    // Close SSH
    sshService.close();
    terminalView.println("\r\n\nSSH: Session closed.");
}

/*
Netcat
*/
void WifiController::handleNetcat(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected())
    {
        terminalView.println("Netcat: You must be connected to Wi-Fi. Use 'connect' first.");
        return;
    }

    // Check args
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (cmd.getSubcommand().empty() || args.size() < 1)
    {
        terminalView.println("Usage: nc <host> <port>");
        return;
    }

    std::string host = cmd.getSubcommand();
    std::string port_str = args[0];

    // Check port
    int port = 1;
    if (argTransformer.isValidNumber(port_str))
    {
        port = argTransformer.parseHexOrDec16(port_str);
    }
    else
    {
        terminalView.println("Netcat: Invalid port number. Use a valid integer.");
        return;
    }

    if (port < 1 || port > 65535)
    {
        terminalView.println("Netcat: Port must be between 1 and 65535.");
        return;
    }

    // Connect, start the netcat task
    terminalView.println("Netcat: Connecting to " + host + " with port " + port_str + "...");
    netcatService.startTask(host, 0, port, true);

    // Wait 5sec for connection success
    unsigned long start = millis();
    while (!netcatService.isConnected() && millis() - start < 5000)
    {
        delay(50);
    }

    // Can't connect
    if (!netcatService.isConnected())
    {
        terminalView.println("\r\nNetcat: Connection failed.");
        netcatService.close();
        return;
    }

    // Connected, start the bridge loop
    terminalView.println("Netcat: Connected. Shell started...\n");
    terminalView.println("Press [CTRL+C],[ESC] or [ANY ESP32 BUTTON] to stop.\n");

    while (true)
    {
        char deviceKey = deviceInput.readChar();
        if (deviceKey != KEY_NONE)
            break;

        char terminalKey = terminalInput.readChar();
        if (terminalKey == KEY_NONE){
            continue;
        }

        netcatService.writeChar(terminalKey);
        terminalView.print(std::string(1, terminalKey));        // local echo
        if (terminalKey == 0x1B || terminalKey == 0x03) break;  // ESC or CTRL+C to exit
        if (terminalKey == '\r' || terminalKey == '\n')
            terminalView.println("");

        std::string output = netcatService.readOutputNonBlocking();
        if (!output.empty())
            terminalView.print(output);

        delay(10);
    }

    // Close Netcat
    netcatService.close();
    terminalView.println("\r\n\nNetcat: Session closed.");
}

/*
Nmap
*/
void WifiController::handleNmap(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected())
    {
        terminalView.println("Nmap: You must be connected to Wi-Fi. Use 'connect' first.");
        return;
    }

    auto args = argTransformer.splitArgs(cmd.getArgs());

    // Parse args
    // Parse hosts first
    auto hosts_arg = cmd.getSubcommand();
    if(!nmapService.parseHosts(hosts_arg)) {
        terminalView.println("Nmap: Invalid host.");
        return;
    }

    // Check the first char of args is '-'
    if (!args.empty() && (args[0].empty() || args[0][0] != '-')) {
        terminalView.println("Nmap: Options must start with '-' (ex: -p 22)");
        return;
    }

    nmapService.setArgTransformer(argTransformer);
    auto tokens = argTransformer.splitArgs(cmd.getArgs());
    auto options = NmapService::parseNmapArgs(tokens);

    if (options.hasTrash){
        // TODO handle this better
        //terminalView.println("Nmap: Invalid options.");
    }

    if (options.hasPort) {
        // Parse ports
        if (!nmapService.parsePorts(options.ports)) {
            terminalView.println("Nmap: invalid -p value. Use 80,22,443 or 1000-2000.");
            return;
        }
        nmapService.setLayer4(options.tcp);
    } else {
        // Set the most popular ports
        nmapService.setDefaultPorts(options.tcp);
        terminalView.println("Nmap: Using top 100 common ports (may take a few seconds)");
    }

    nmapService.startTask(options.verbosity);
    
    while(!nmapService.isReady()){
        delay(100);
    }

    terminalView.println(nmapService.getReport());
    nmapService.clean();
    
    terminalView.println("\r\n\nNmap: Scan finished.");
}

/*
Config
*/
void WifiController::handleConfig()
{
    terminalView.println("[WARNING] If you're connected via Web CLI,");
    terminalView.println("          executing Wi-Fi commands may cause ");
    terminalView.println("          the terminal session to disconnect.");
    terminalView.println("          Don't use: sniff, probe, connect, scan, spoof...");
    terminalView.println("          Use USB serial or restart if connection is lost.\n");
}

/*
Help
*/
void WifiController::handleHelp()
{
    terminalView.println("WiFi commands:");
    terminalView.println("  scan");
    terminalView.println("  connect");
    terminalView.println("  ping <host>");
    terminalView.println("  sniff");
    terminalView.println("  probe");
    terminalView.println("  spoof sta <mac>");
    terminalView.println("  spoof ap <mac>");
    terminalView.println("  status");
    terminalView.println("  disconnect");
    terminalView.println("  ap <ssid> <password>");
    terminalView.println("  ssh <host> <username> <password> [port]");
    terminalView.println("  nc <host> <port>");
    terminalView.println("  nmap <host> [port]");
    terminalView.println("  webui");
    terminalView.println("  reset");
    terminalView.println("  deauth <ssid>");
}

/*
Ensure Configuration
*/
void WifiController::ensureConfigured()
{
    if (!configured)
    {
        handleConfig();
        configured = true;
    }
}

/*
Deauthenticate stations attack
*/
void WifiController::handleDeauth(const TerminalCommand &cmd)
{
    auto target = cmd.getSubcommand();

    // if the SSID have space in name, e.g "Router Wifi"
    if (!cmd.getArgs().empty())
    {
        target += " " + cmd.getArgs();
    }

    if (target.empty())
    {
        terminalView.println("Usage: deauth <ssid>");
        return;
    }

    terminalView.println("WiFi: Sending deauth to \"" + target + "\"...");

    bool ok = wifiService.deauthApBySsid(target);

    if (ok)
        terminalView.println("WiFi: Deauth frames sent.");
    else
        terminalView.println("WiFi: SSID not found.");
}
