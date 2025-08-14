#pragma once

#include <Arduino.h>
#include <string>
#include <vector>
#include <array>

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/EthernetService.h"
#include "Services/SshService.h"
#include "Services/NetcatService.h"
#include "Services/NmapService.h"
#include "Transformers/ArgTransformer.h"
#include "Models/TerminalCommand.h"
#include "States/GlobalState.h"
#include "Managers/UserInputManager.h"

class EthernetController {
public:
    EthernetController(ITerminalView& terminalView,
                       IInput& terminalInput,
                       IInput& deviceInput,
                       EthernetService& ethernetService,
                       ArgTransformer& argTransformer,
                       UserInputManager& userInputManager);

    // Entry point for Ethernet command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure Ethernet is configured before any action
    void ensureConfigured();

private:
    // Handlers
    void handleConfig();
    void handleStatus();
    void handleConnect();
    void handleReset();
    void handleHelp();

private:
    ITerminalView& terminalView;
    IInput&        terminalInput;
    IInput&        deviceInput;

    EthernetService& ethernetService;
    ArgTransformer&  argTransformer;
    UserInputManager& userInputManager;
    
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};
