// InfraredController.h
#pragma once

#include <sstream>
#include <string>
#include <algorithm>
#include <Arduino.h>

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/InfraredService.h"
#include "Models/TerminalCommand.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Shells/UniversalRemoteShell.h"

class InfraredController {
public:
    // Constructor
    InfraredController(ITerminalView& view, IInput& terminalInput, 
                       InfraredService& service, ArgTransformer& ArgTransformer,
                       UserInputManager& userInputManager, UniversalRemoteShell& universalRemoteShell);

    // Entry point for Infraredcommand dispatch
    void handleCommand(const TerminalCommand& command);

    // Ensure infrared is properly configured
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    InfraredService& infraredService;
    GlobalState& state = GlobalState::getInstance();
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    UniversalRemoteShell& universalRemoteShell;
    bool configured = false;
    uint8_t MAX_IR_FRAMES = 64; // Maximum frames to record


    // Frames
    struct IRFrame {
        std::vector<uint16_t> timings; // raw ir timings
        uint32_t khz; // carrier frequency
        uint32_t gapMs; // delay from previous frame in milliseconds
    };

    // Configure IR settings
    void handleConfig();

    // Send IR command
    void handleSend(const TerminalCommand& command);

    // Receive IR commands
    void handleReceive();

    // Send "device-b-gone" style power-off signals
    void handleDeviceBgone();

    // Set IR protocol
    void handleSetProtocol();

    // Universal remote shell
    void handleRemote();

    // Handle replay of IR commands
    void handleReplay(const TerminalCommand& command);
    bool recordFrames(std::vector<IRFrame>& tape);
    void playbackFrames(const std::vector<IRFrame>& tape, uint32_t replayCount);

    // Show help text
    void handleHelp();
};
