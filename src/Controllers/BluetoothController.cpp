#include "Controllers/BluetoothController.h"

/*
Constructor
*/
BluetoothController::BluetoothController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    BluetoothService& bluetoothService,
    ArgTransformer& argTransformer
) : terminalView(terminalView),
    terminalInput(terminalInput),
    bluetoothService(bluetoothService),
    argTransformer(argTransformer) {}

/*
Entry point for BT command
*/
void BluetoothController::handleCommand(const TerminalCommand& cmd) {
    const auto& root = cmd.getRoot();

    if (root == "scan") {
        handleScan();
    } else if (root == "pair") {
        handlePair(cmd);
    } else if (root == "spoof") {
        handleSpoof(cmd);
    } else if (root == "sniff") {
        handleSniff(cmd);
    } else if (root == "status") {
        handleStatus();
    } else if (root == "server") {
        handleServer(cmd);
    } else if (root == "keyboard") {
        handleKeyboard(cmd);
    } else if (root == "mouse") {
        handleMouse(cmd);
    } else if (root == "reset") {
        handleReset();
    } else {
        handleHelp();
    }
}

/*
Scan
*/
void BluetoothController::handleScan() {
    terminalView.println("Bluetooth Scan: In progress for 10 sec...\n");

    auto lines = bluetoothService.scanDevices(10);
    if (lines.empty()) {
        terminalView.println("Bluetooth Scan: No devices");
        return;
    }

    for (const auto& line : lines) {
        terminalView.println("  " + line + "\n");
    }
}

/*
Pair
*/
void BluetoothController::handlePair(const TerminalCommand& cmd) {
    bluetoothService.switchToMode(BluetoothMode::CLIENT);
    std::string addr = cmd.getSubcommand();
    if (addr.empty()) {
        terminalView.println("Usage: pair <mac address>");
        return;
    }

    terminalView.println("Bluetooth Pair: Attempting " + addr + "...");

    auto services = bluetoothService.connectTo(addr);
    if (!services.empty()) {
        terminalView.println("Bluetooth Pair: Successfully connected.");
        terminalView.println("Bluetooth Pair: Services discovered:");
        for (const auto& uuid : services) {
            terminalView.println("  - " + uuid);
        }
    } else {
        terminalView.println("Bluetooth Pair: Failed to connect to " + addr);
    }
}

/*
Status
*/
void BluetoothController::handleStatus() {
    terminalView.println("Bluetooth Status:");

    if (bluetoothService.getMode() == BluetoothMode::NONE) {
        terminalView.println("  Mode: Not initialized");
        return;
    } else if (bluetoothService.getMode() == BluetoothMode::CLIENT) {
        terminalView.println("  Mode: Client");
    } else {
        terminalView.println("  Mode: Server");
    }

    terminalView.println("  Connected: " + std::string(bluetoothService.isConnected() ? "Yes" : "No"));

    std::string mac = bluetoothService.getMacAddress();
    if (!mac.empty()) {
        terminalView.println("  MAC Address: " + mac);
    } else {
        terminalView.println("  MAC Address: Unknown");
    }
}

/*
Sniff
*/
void BluetoothController::handleSniff(const TerminalCommand& cmd) {
    terminalView.println("Bluetooth Sniff: Started... Press [ENTER] to stop.\n");

    bluetoothService.switchToMode(BluetoothMode::CLIENT);
    BluetoothService::startPassiveBluetoothSniffing();

    unsigned long lastPull = 0;

    while (true) {
        // Enter press
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        // Show paquets if any
        if (millis() - lastPull > 200) { 
            auto logs = BluetoothService::getBluetoothSniffLog();
            for (const auto& line : logs) {
                terminalView.println(line);
            }
            lastPull = millis();
        }

        delay(10);
    }

    BluetoothService::stopPassiveBluetoothSniffing();
    terminalView.println("Bluetooth Sniff: Stopped by user.\n");
}

/*
Server
*/
void BluetoothController::handleServer(const TerminalCommand& cmd) {
    if (bluetoothService.getMode() == BluetoothMode::SERVER && bluetoothService.isConnected()) {
        terminalView.println("Bluetooth Server: Already Started");
        return;
    }

    std::string name = cmd.getSubcommand();
    if (name.empty()) {
        name = "Bus-Pirate-Bluetooth";
    }

    terminalView.println("Bluetooth Server: Starting BLE HID server as \"" + name + "\"...");
    bluetoothService.begin(name);
    terminalView.println("→ You can now pair from your phone or computer.");
}

/*
Keyboard
*/
void BluetoothController::handleKeyboard(const TerminalCommand& cmd) {
    if (bluetoothService.getMode() != BluetoothMode::SERVER) {
        terminalView.println("Bluetooth Keyboard: Start the server before sending data");
        return;
    }

    auto text = cmd.getSubcommand();
    if (text.empty()) {
        terminalView.println("Usage: keyboard <text>");
        return;
    }

    bluetoothService.sendKeyboardText(text);
    terminalView.println("Bluetooth Keyboard: String sent.");
}

/*
Mouse
*/
void BluetoothController::handleMouse(const TerminalCommand& cmd) {
    if (bluetoothService.getMode() != BluetoothMode::SERVER) {
        terminalView.println("Bluetooth Mouse: Start the server before sending data");
        return;
    }

    // mouse click
    if (cmd.getSubcommand() == "click") {
        bluetoothService.clickMouse();
        terminalView.println("Bluetooth Mouse: Click sent.");
        return;
    }

    auto args = argTransformer.splitArgs(cmd.getArgs());

    // mouse move x y
    if (args.size() == 2 && cmd.getSubcommand() == "move" &&
        argTransformer.isValidSignedNumber(args[0]) &&
        argTransformer.isValidSignedNumber(args[1])) {

        int8_t x = argTransformer.toClampedInt8(args[0]);
        int8_t y = argTransformer.toClampedInt8(args[1]);

        bluetoothService.mouseMove(x, y);
        terminalView.println("Bluetooth Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        return;
    }
    
    // mouse x y
    if (args.size() != 1 ||
        !argTransformer.isValidSignedNumber(cmd.getSubcommand()) ||
        !argTransformer.isValidSignedNumber(args[0])) {
        terminalView.println("Usage: mouse <x> <y> or mouse click");
        return;
    }

    int8_t x = argTransformer.toClampedInt8(cmd.getSubcommand());
    int8_t y = argTransformer.toClampedInt8(args[0]);

    bluetoothService.mouseMove(x, y);
    terminalView.println("Bluetooth Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

/*
Spoof
*/
void BluetoothController::handleSpoof(const TerminalCommand& cmd) {
    std::string mac = cmd.getSubcommand();
    if (bluetoothService.isConnected() || bluetoothService.getMode() != BluetoothMode::NONE) {
        terminalView.println("Bluetooth Spoof: You must set the address before init Bluetooth. Use 'reset' command");
        return;
    }

    if (mac.empty()) {
        terminalView.println("Usage: spoof <mac address>");
        return;
    }

    bool success = bluetoothService.spoofMacAddress(mac);
    if (success) {
        terminalView.println("Bluetooth Spoof: MAC address overridden to " + mac);
    } else {
        terminalView.println("Bluetooth Spoof: Failed to set MAC address.");
    }
}

/*
Reset
*/
void BluetoothController::handleReset() {
    bluetoothService.end();
    terminalView.println("Bluetooth: Reset complete.");
}

/*
Config
*/
void BluetoothController::handleConfig() {
    // bluetoothService.init();
}

/*
Help
*/
void BluetoothController::handleHelp() {
    terminalView.println("Bluetooth commands:");
    terminalView.println("  scan");
    terminalView.println("  pair <addr>");
    terminalView.println("  spoof <mac>");
    terminalView.println("  sniff");
    terminalView.println("  status");
    terminalView.println("  server");
    terminalView.println("  keyboard <text>");
    terminalView.println("  mouse <x> <y>");
    terminalView.println("  mouse click");
    terminalView.println("  reset");
}

/*
Ensure Config
*/
void BluetoothController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
