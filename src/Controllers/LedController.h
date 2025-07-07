#pragma once

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Services/LedService.h"

// Placeholder implementation for LED controller
// Future versions may support RGB patterns, animations, or PWM control

class LedController {
public:
    // Constructor
    LedController(ITerminalView& terminalView, IInput& terminalInput, LedService& ledService);

    // Entry point for dispatch incoming LED command
    void handleCommand(const TerminalCommand& cmd);

    //  Entry point for handle bytecode-style instructions (parsed instructions)
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure LED is properly configured before executing commands
    void ensureConfigured();

private:
    // Run LED configuration routine (e.g. default pin setup)
    void handleConfig();

    ITerminalView& terminalView;     // Output view for terminal
    IInput& terminalInput;          // Input handler
    LedService& ledService;        // Service for low-level LED operations
    bool configured = false;      // Whether the controller is configured
};
