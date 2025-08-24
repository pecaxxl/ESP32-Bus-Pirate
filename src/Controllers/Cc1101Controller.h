// CC1101Controller.h
#pragma once

#include <string>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/Cc1101Service.h"
#include "Models/TerminalCommand.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"

class CC1101Controller {
public:
    // Constructor
    CC1101Controller(ITerminalView& terminalView, IInput& terminalInput, CC1101Service& cc1101Service, ArgTransformer& argTransformer, UserInputManager& userInputManager);

    // Entry point to handle a DIO command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure configuration is done before running commands
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    CC1101Service& cc1101Service;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
    UserInputManager& userInputManager;

    bool configured = false;

    // Read digital value from a pin
    void handleReadPin(const TerminalCommand& cmd);
    void handleSend(TerminalCommand cmd);
    void handleRXraw(const TerminalCommand& cmd);
    void handleTXraw(const TerminalCommand& cmd);
    
    // Available commands
    void handleHelp();
    void handleConfig();
};