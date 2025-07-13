#pragma once

// Placeholder implementation for SPI controller

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Data/FlashDatabase.h"

class SpiController {
public:
    // Constructor
    SpiController(ITerminalView& terminalView, IInput& terminalInput, SpiService& service, ArgTransformer& argTransformer, UserInputManager& userInputManager); 

    // Entry point for handle raw user command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for handle parsed instruction bytecodes
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure SPI is properly configured before use
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    SpiService& spiService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;

    // Passive bus monitor
    void handleSniff();

    // Probe flash chip and identify it
    void handleFlashProbe();

    // Read data from flash memory
    void handleFlashRead(const TerminalCommand& cmd);
    void readFlashInChunks(uint32_t address, uint32_t length);

    // Write data to flash memory
    void handleFlashWrite();

    // Erase flash memory
    void handleFlashErase(const TerminalCommand& cmd);

    // Configure SPI bus parameters
    void handleConfig();

};
