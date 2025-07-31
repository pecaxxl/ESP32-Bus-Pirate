#pragma once

// Placeholder implementation for 2WIRE controller

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Services/TwoWireService.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"

class TwoWireController {
public:
    // Constructor
    TwoWireController(ITerminalView& terminalView, IInput& terminalInput, UserInputManager& userInputManager, TwoWireService& twoWireService);

    // Entry point for handle raw user command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for handle compiled bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure 2WIRE is configured before use
    void ensureConfigured();

private:
    // Sniff 2wire bus
    void handleSniff();

    // Smartcard commands
    void handleSmartCard(const TerminalCommand& cmd);

    // Perform a smartcard Answer To Reset and decode it
    void handleSmartCardProbe();

    // Display smartcard security memory (left attempts)
    void handleSmartCardSecurity();

    // Dump the 256 bytes content of main memory + sec/prt memory
    void handleSmartCardDump();

    // Unlock smartcard memory with the PSC (PIN code)
    void handleSmartCardUnlock();

    // Protect smartcard memory permanently
    void handleSmartCardProtect();

    // PSC (Pin code) commands
    void handleSmartCardPsc(const TerminalCommand& cmd);

    // Write a byte at given address in main memory
    void handleSmartCardWrite(const TerminalCommand& cmd);

    // User pin configuration
    void handleConfig();

    // Show available commands
    void handleHelp();

    ITerminalView& terminalView;
    IInput& terminalInput;
    UserInputManager& userInputManager;
    TwoWireService& twoWireService;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
};
