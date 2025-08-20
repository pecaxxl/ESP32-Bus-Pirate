#pragma once

#include <Interfaces/ITerminalView.h>
#include <Interfaces/IInput.h>
#include <Services/WifiService.h>
#include <Services/NvsService.h>
#include <Services/SshService.h>
#include <Services/NetcatService.h>
#include <Services/NmapService.h>
#include <Services/ICMPService.h>
#include <Services/WifiOpenScannerService.h>
#include <Transformers/ArgTransformer.h>
#include <Managers/UserInputManager.h>
#include <Models/TerminalCommand.h>
#include <States/GlobalState.h>
#include <Abstracts/ANetworkController.h>
#include <Preferences.h>

class WifiController : public ANetworkController {
public:
    using ANetworkController::ANetworkController;

    //  Entry point for Wi-Fi command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure WiFi is configured before any action
    void ensureConfigured();

private:
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
    Preferences preferences;

    // Handle connection to a Wi-Fi network
    void handleConnect(const TerminalCommand& cmd);

    // Handle disconnection from current Wi-Fi
    void handleDisconnect(const TerminalCommand& cmd);

    // Display current Wi-Fi status
    void handleStatus(const TerminalCommand& cmd);

    // Configure and start Access Point mode
    void handleAp(const TerminalCommand& cmd);

    // Spam random acccess point
    void handleApSpam();

    // Perform Wi-Fi network scan
    void handleScan(const TerminalCommand& cmd);

    // Start probing open networks for net access
    void handleProbe();

    // Start packet sniffing
    void handleSniff(const TerminalCommand& cmd);

    // Show WebUI IP
    void handleWebUi(const TerminalCommand& cmd);

    // Spoof Mac addr
    void handleSpoof(const TerminalCommand& cmd);

    // Deauthentication attack
    void handleDeauth(const TerminalCommand& cmd);

    // Reset Wi-Fi configuration
    void handleReset();

    // Ask config for Wi-Fi
    void handleConfig();

    // Show help for Wi-Fi commands
    void handleHelp();
};
