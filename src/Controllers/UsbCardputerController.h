#pragma once

#ifdef DEVICE_CARDPUTER

#include <sstream>
#include <Arduino.h>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Interfaces/IUsbController.h"
#include "Interfaces/IUsbService.h"

class UsbCardputerController: public IUsbController {
public:
    // Constructor
    UsbCardputerController(ITerminalView& terminalView, IInput& terminalInput, IUsbService& usbService, ArgTransformer& argTransformer, UserInputManager& userInputManager);

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
    IUsbService& usbService;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
    UserInputManager& userInputManager;
};

#endif