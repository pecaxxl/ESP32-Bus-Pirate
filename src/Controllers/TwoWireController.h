#pragma once

// Placeholder implementation for 2WIRE controller

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"

class TwoWireController {
public:
    // Constructor
    TwoWireController(ITerminalView& terminalView, IInput& terminalInput);

    // Entry point for handle raw user command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for handle compiled bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure 2WIRE is configured before use
    void ensureConfigured();

private:
    ITerminalView& terminalView;  // Terminal output
    IInput& terminalInput;       // User input
    bool configured = false;     // Init state

    // Start sniffing 2WIRE bus
    void handleSniff();

    // Set up 2WIRE parameters
    void handleConfig();
};
