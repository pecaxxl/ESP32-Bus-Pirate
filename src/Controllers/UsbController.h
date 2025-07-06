#pragma once

#include <sstream>
#include "Views/ITerminalView.h"
#include "Inputs/IInput.h"
#include "Services/UsbService.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"

class UsbController {
public:
    // Constructor
    UsbController(ITerminalView& terminalView, IInput& terminalInput, UsbService& usbService, ArgTransformer& argTransformer, UserInputManager& userInputManager);

    // Entry point for handle raw terminal command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure USB is configured before any action
    void ensureConfigured();

private:
    // Simulate USB mass storage mount
    void handleUsbStick();

    // Send keyboard input via HID
    void handleKeyboardSend(const TerminalCommand& cmd);

    // Move the HID mouse cursor
    void handleMouseMove(const TerminalCommand& cmd);

    // Perform HID mouse click
    void handleMouseClick();

    // High-level mouse handler
    void handleMouse(const TerminalCommand& cmd);

    // Emulate gamepad events
    void handleGamepad(const TerminalCommand& cmd);

    // Reset all USB states
    void handleReset();

    // Configure USB behavior
    void handleConfig();

    // Display help for USB commands
    void handleHelp();

    // Prompt user input via terminal
    std::string getUserInput();

    bool configured = false;
    ITerminalView& terminalView;
    IInput& terminalInput;
    UsbService& usbService;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
    UserInputManager& userInputManager;
};
