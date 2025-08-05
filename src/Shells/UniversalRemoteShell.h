#pragma once

#include <Arduino.h>
#include "Services/InfraredService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"

class UniversalRemoteShell {
public:
    UniversalRemoteShell( ITerminalView& view, IInput& input, InfraredService& irService, ArgTransformer& argTransformer, UserInputManager& userInputManager);
    void run();

private:
    InfraredService& infraredService;
    ITerminalView& terminalView;
    IInput& terminalInput;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;

    void sendCommandGroup(const std::vector<InfraredCommand>& group);
};
