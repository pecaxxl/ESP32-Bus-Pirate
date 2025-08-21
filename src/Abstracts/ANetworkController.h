#pragma once

#include <Arduino.h>
#include <string>
#include <ESP32Ping.h>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/WifiService.h"
#include "Services/WifiOpenScannerService.h"
#include "Services/EthernetService.h"
#include "Services/SshService.h"
#include "Services/NetcatService.h"
#include "Services/NmapService.h"
#include "Services/ICMPService.h"
#include "Services/NvsService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Models/TerminalCommand.h"

class ANetworkController {
public:

    ANetworkController(
        ITerminalView& terminalView, 
        IInput& terminalInput, 
        IInput& deviceInput,
        WifiService& wifiService, 
        WifiOpenScannerService& wifiOpenNetworkService,
        EthernetService& ethernetService,
        SshService& sshService,
        NetcatService& netcatService,
        NmapService& nmapService,
        ICMPService& icmpService,
        NvsService& nvsService, 
        ArgTransformer& argTransformer,
        UserInputManager& userInputManager
    );

protected:
    void handleNetcat(const TerminalCommand& cmd);
    void handleNmap(const TerminalCommand& cmd);
    void handleSsh(const TerminalCommand& cmd);
    void handlePing(const TerminalCommand& cmd);
    void handleDiscovery(const TerminalCommand &cmd);

protected:
    ITerminalView&     terminalView;
    IInput&            terminalInput;
    IInput&            deviceInput;

    WifiService&       wifiService;
    EthernetService&   ethernetService;

    NvsService&        nvsService;

    WifiOpenScannerService& wifiOpenScannerService;
    SshService&        sshService;
    NetcatService&     netcatService;
    NmapService&       nmapService;
    ICMPService&       icmpService;
    
    ArgTransformer&    argTransformer;
    UserInputManager&  userInputManager;
    GlobalState&       globalState = GlobalState::getInstance();
};
