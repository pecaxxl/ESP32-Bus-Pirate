#pragma once

// Placeholder implementation for SPI controller

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Services/SdService.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Managers/BinaryAnalyzeManager.h"
#include "Managers/SdCardManager.h"
#include "States/GlobalState.h"
#include "Data/FlashDatabase.h"

class SpiController {
public:
    // Constructor
    SpiController(ITerminalView& terminalView, IInput& terminalInput, SpiService& spiService, SdService& sdService, ArgTransformer& argTransformer, UserInputManager& userInputManager, BinaryAnalyzeManager& binaryAnalyzeManager, SdCardManager& sdCardManager); 

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
    SdService& sdService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    BinaryAnalyzeManager& binaryAnalyzeManager;
    SdCardManager& sdCardManager;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;

    // Passive bus monitor
    void handleSniff();

    // Probe flash chip and identify it
    void handleFlashProbe();

    // Analyze flash content
    void handleFlashAnalyze(const TerminalCommand& cmd);

    // Extract strings from the flash content
    void handleFlashStrings(const TerminalCommand& cmd);

    // Search pattern into the flash content
    void handleFlashSearch(const TerminalCommand& cmd);

    // Read data from flash memory
    void handleFlashRead(const TerminalCommand& cmd);
    void readFlashInChunks(uint32_t address, uint32_t length);

    // Write data to flash memory
    void handleFlashWrite(const TerminalCommand& cmd);

    // Erase flash memory
    void handleFlashErase(const TerminalCommand& cmd);

    // Check before any flash action
    bool checkFlashPresent();

    // SD operations
    void handleSdCard();

    // Slave mode
    void handleSlave();

    // Configure SPI bus parameters
    void handleConfig();

    // Available commands
    void handleHelp();


};
