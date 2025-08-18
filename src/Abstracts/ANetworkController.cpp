#include "ANetworkController.h"
#include <ESP32Ping.h>


ANetworkController::ANetworkController(
    ITerminalView& terminalView, 
    IInput& terminalInput, 
    IInput& deviceInput,
    WifiService& wifiService, 
    WifiOpenScannerService& wifiOpenScannerService,
    EthernetService& ethernetService,
    SshService& sshService,
    NetcatService& netcatService,
    NmapService& nmapService,
    NvsService& nvsService, 
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager
)
: terminalView(terminalView),
  terminalInput(terminalInput),
  deviceInput(deviceInput),
  wifiService(wifiService),
  wifiOpenScannerService(wifiOpenScannerService),
  ethernetService(ethernetService),
  sshService(sshService),
  netcatService(netcatService),
  nmapService(nmapService),
  nvsService(nvsService),
  argTransformer(argTransformer),
  userInputManager(userInputManager)
{
}

/*
Ping
*/
void ANetworkController::handlePing(const TerminalCommand &cmd)
{
    if (!wifiService.isConnected() && !ethernetService.isConnected()) {
        terminalView.println("Ping: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    const std::string host = cmd.getSubcommand();
    if (host.empty()) {
        terminalView.println("Usage: ping <host|ip>");
        return;
    }

    // Laisse la lib gérer IP vs DNS :
    const unsigned long t0 = millis();
    const bool ok = Ping.ping(host.c_str(), 1);
    const unsigned long t1 = millis();

    if (ok) {
        const int avg = Ping.averageTime();
        terminalView.println("Ping: " + host + " ok (" + std::to_string(avg) + " ms).");
    } else {
        terminalView.println("Ping: Failed to ping " + host + ".");
    }
}

/*
Netcat
*/
void ANetworkController::handleNetcat(const TerminalCommand& cmd)
{
    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("Netcat: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }
    // Args: nc <host> <port>
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (cmd.getSubcommand().empty() || args.size() < 1) {
        terminalView.println("Usage: nc <host> <port>");
        return;
    }

    std::string host = cmd.getSubcommand();
    std::string portStr = args[0];

    if (!argTransformer.isValidNumber(portStr)) {
        terminalView.println("Netcat: Invalid port number.");
        return;
    }
    int port = argTransformer.parseHexOrDec16(portStr);
    if (port < 1 || port > 65535) {
        terminalView.println("Netcat: Port must be between 1 and 65535.");
        return;
    }

    terminalView.println("Netcat: Connecting to " + host + " with port " + portStr + "...");
    netcatService.startTask(host, 0, port, true);

    // Wait for connection
    unsigned long start = millis();
    while (!netcatService.isConnected() && millis() - start < 5000) {
        delay(50);
    }

    if (!netcatService.isConnected()) {
        terminalView.println("\r\nNetcat: Connection failed.");
        netcatService.close();
        return;
    }

    terminalView.println("Netcat: Connected. Shell started...\n");
    terminalView.println("Press [CTRL+C],[ESC] or [ANY ESP32 BUTTON] to stop.\n");

    while (true) {
        char deviceKey = deviceInput.readChar();
        if (deviceKey != KEY_NONE)
            break;

        char terminalKey = terminalInput.readChar();
        if (terminalKey == KEY_NONE) {
            std::string out = netcatService.readOutputNonBlocking();
            if (!out.empty()) terminalView.print(out);
            delay(10);
            continue;
        }

        netcatService.writeChar(terminalKey);
        terminalView.print(std::string(1, terminalKey));        // local echo
        if (terminalKey == 0x1B || terminalKey == 0x03) break;  // ESC ou CTRL+C
        if (terminalKey == '\r' || terminalKey == '\n') terminalView.println("");

        std::string output = netcatService.readOutputNonBlocking();
        if (!output.empty()) terminalView.print(output);
        delay(10);
    }

    netcatService.close();
    terminalView.println("\r\n\nNetcat: Session closed.");
}

/*
Nmap
*/
void ANetworkController::handleNmap(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("Nmap: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
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
SSH
*/
void ANetworkController::handleSsh(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("SSH: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
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