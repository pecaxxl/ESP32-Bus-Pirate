#pragma once

#include <string>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/BluetoothService.h"
#include "Transformers/ArgTransformer.h"
#include "Models/TerminalCommand.h"

class BluetoothController {
public:
    BluetoothController(
        ITerminalView& terminalView,
        IInput& terminalInput,
        BluetoothService& bluetoothService,
        ArgTransformer& argTransformer
    );

    // Entry point for BT command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure BT configuration
    void ensureConfigured();
    
private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    BluetoothService& bluetoothService;
    ArgTransformer& argTransformer;
    bool configured = false;
    
    // Scan for BT devices
    void handleScan();

    // Pair with BT device
    void handlePair(const TerminalCommand& cmd);

    // Spoof mac addr
    void handleSpoof(const TerminalCommand& cmd);

    // Show BT status
    void handleStatus();

    // Create BT HID server
    void handleServer(const TerminalCommand& cmd);

    // Handle KB action
    void handleKeyboard(const TerminalCommand& cmd);

    // Handle mouse action
    void handleMouse(const TerminalCommand& cmd);

    // Sniff BT server I/O
    void handleSniff();

    // Available BT commands
    void handleHelp();

    // Config BT
    void handleConfig();

    // Reset BT interface
    void handleReset();
};