#pragma once

#include <vector>
#include <Arduino.h>
#include <string>
#include <sstream>
#include <algorithm>
#include "Models/TerminalCommand.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "States/GlobalState.h"
#include "Enums/ModeEnum.h"
#include "Services/PinService.h"
#include "Managers/UserInputManager.h"

class UtilityController {
public:
    // Constructor
    UtilityController(ITerminalView& terminalView, IInput& terminalInput, 
                      PinService& pinService, UserInputManager& userInputManager);

    // Entry point for global utility commands
    void handleCommand(const TerminalCommand& cmd);

    // Process mode change command and return new mode
    ModeEnum handleModeChangeCommand(const TerminalCommand& cmd);

    // Check if a command is a global utility command
    bool isGlobalCommand(const TerminalCommand& cmd);

private:
    // Display help for utility commands
    void handleHelp();

    // Ask user to select a mode
    ModeEnum handleModeSelect();

    // Enable internal pull-up resistors
    void handleEnablePullups();

    // Disable internal pull-up resistors
    void handleDisablePullups();

    ITerminalView& terminalView;
    IInput& terminalInput;
    PinService& pinService;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();
};