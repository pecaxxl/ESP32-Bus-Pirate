#pragma once

#include "Views/ITerminalView.h"
#include "Inputs/IInput.h"
#include "Models/ByteCode.h"
#include "Models/TerminalCommand.h"
#include "Services/HdUartService.h"
#include "Transformers/ArgTransformer.h"
#include "States/GlobalState.h"


class HdUartController {
public:
    HdUartController(ITerminalView& terminalView, IInput& terminalInput, IInput& deviceInput,
                     HdUartService& hdUartService, ArgTransformer& argTransformer);
    
    // Entry point for HDUART command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for HDUART instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // HDUART config check
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    IInput& deviceInput;
    HdUartService& hdUartService;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
    
    bool configured = false;
    
    // HDUART Bridge mode read/write on one line
    void handleBridge();

    // Configure HDUART
    void handleConfig();

    // HDURT Available commands
    void handleHelp();

    std::string getUserInput();
    uint8_t readValidatedUint8(const std::string& label, uint8_t defaultVal, uint8_t min = 0, uint8_t max = 255);
    uint32_t readValidatedUint32(const std::string& label, uint32_t defaultVal);
    char readCharChoice(const std::string& label, char defaultVal, const std::vector<char>& allowed);
    bool readYesNo(const std::string& label, bool defaultVal);
    uint32_t buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits);
};
