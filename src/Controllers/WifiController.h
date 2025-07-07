#pragma once

#include <Interfaces/ITerminalView.h>
#include <Interfaces/IInput.h>
#include <Services/WifiService.h>
#include <Services/NvsService.h>
#include <Transformers/ArgTransformer.h>
#include <Models/TerminalCommand.h>
#include <States/GlobalState.h>

class WifiController {
public:
    // Constructor
    WifiController(ITerminalView& terminalView, IInput& terminalInput, WifiService& wifiService, NvsService& nvsService, ArgTransformer& argTransformer);

    //  Entry point for Wi-Fi command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure WiFi is configured before any action
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    WifiService& wifiService;
    NvsService& nvsService;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;

    // Handle connection to a Wi-Fi network
    void handleConnect(const TerminalCommand& cmd);

    // Handle disconnection from current Wi-Fi
    void handleDisconnect(const TerminalCommand& cmd);

    // Display current Wi-Fi status
    void handleStatus(const TerminalCommand& cmd);

    // Configure and start Access Point mode
    void handleAp(const TerminalCommand& cmd);

    // Perform Wi-Fi network scan
    void handleScan(const TerminalCommand& cmd);

    // Ping a target over Wi-Fi
    void handlePing(const TerminalCommand& cmd);

    // Start packet sniffing
    void handleSniff(const TerminalCommand& cmd);

    // Show WebUI IP
    void handleWebUi(const TerminalCommand& cmd);

    // Reset Wi-Fi configuration
    void handleReset();

    // Ask config for Wi-Fi
    void handleConfig();

    // Show help for Wi-Fi commands
    void handleHelp();

};
