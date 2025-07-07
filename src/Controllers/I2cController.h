#pragma once

#include <sstream> 
#include <string>
#include <algorithm>
#include "Views/ITerminalView.h"
#include "Inputs/IInput.h"
#include "Services/I2cService.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Vendors/i2c_sniffer.h"

struct I2cSniffState;

class I2cController {
public:
    // Constructor
    I2cController(ITerminalView& terminalView, IInput& terminalInput, I2cService& i2cService, ArgTransformer& argTransformer, UserInputManager& userInputManager);

    // Entry point for I2C command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for compiled bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure I2C is configured before use
    void ensureConfigured();
    
private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    I2cService& i2cService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
    
    // Ping an I2C address
    void handlePing(const TerminalCommand& cmd);

    // Scan the I2C bus for devices
    void handleScan();

    // Start sniffing I2C traffic passively
    void handleSniff();

    // Read data from an I2C device
    void handleRead(const TerminalCommand& cmd);

    // Write data to an I2C device
    void handleWrite(const TerminalCommand& cmd);

    // Configure I2C parameters
    void handleConfig();

    // Show I2C help message
    void handleHelp();

    // Read user input from terminal
    std::string getUserInput();
};
