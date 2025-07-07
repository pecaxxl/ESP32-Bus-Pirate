#pragma once

// Placeholder implementation for 3WIRE controller
// Functionality to be added in future versions

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"

class ThreeWireController {
public:
    // Constructor
    ThreeWireController(ITerminalView& terminalIiew, IInput& terminalInput);

    // Entry point for handle raw user command
    void handleCommand(const TerminalCommand& cmd);

    // Handle compiled bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure 3WIRE is configured before use
    void ensureConfigured();

private:
    ITerminalView& terminalView;  // Terminal output
    IInput& terminalInput;       // User input
    bool configured = false;     // Init state

    // Set up 3WIRE parameters
    void handleConfig();
};
