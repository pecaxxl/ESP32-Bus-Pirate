#pragma once

#include <Arduino.h>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/I2sService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Models/TerminalCommand.h"
#include "States/GlobalState.h"

class I2sController {
public:
    I2sController(ITerminalView& terminalView, IInput& terminalInput,
                  I2sService& i2sService, ArgTransformer& argTransformer,
                  UserInputManager& userInputManager);

    void handleCommand(const TerminalCommand& cmd);
    void ensureConfigured();

private:
    void handleConfig();
    void handlePlay(const TerminalCommand& cmd);
    void handleRecord(const TerminalCommand& cmd);
    void handleReset();
    void handleHelp();

    ITerminalView& terminalView;
    IInput& terminalInput;
    I2sService& i2sService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
};
