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
    ITerminalView& terminalView;  // Terminal output
    IInput& terminalInput;       // User input
    UserInputManager& userInputManager;
    TwoWireService& twoWireService;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;     // Init state

    // Command handlers
    void handleSec();
    void handleATR();
    void handleConfig();
    void handleHelp();
    void handleTest();
    void handleDump();
};
