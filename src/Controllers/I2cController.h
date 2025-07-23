#pragma once

#include <sstream> 
#include <string>
#include <algorithm>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
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

    // Emulate I2C slave device logging master command
    void handleSlave(const TerminalCommand& cmd);

    // Attempt to glitch an I2C device
    void handleGlitch(const TerminalCommand& cmd);

    // Flood an I2C device with commands
    void handleFlood(const TerminalCommand& cmd);

    // Configure I2C parameters
    void handleConfig();

    // Show I2C help message
    void handleHelp();

    // Dump I2C registers content
    void handleDump(const TerminalCommand& cmd);
    void performRegisterRead(uint8_t addr, uint16_t, uint16_t len,
                            std::vector<uint8_t>& values, std::vector<bool>& valid);
    void performRawRead(uint8_t addr, uint16_t, uint16_t len,
                        std::vector<uint8_t>& values, std::vector<bool>& valid);
    void printHexDump(uint16_t, uint16_t len,
                    const std::vector<uint8_t>& values, const std::vector<bool>& valid);
};
