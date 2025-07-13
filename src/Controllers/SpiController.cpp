#include "SpiController.h"

SpiController::SpiController(ITerminalView& terminalView, IInput& terminalInput, 
                             SpiService& service, ArgTransformer& argTransformer,
                             UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), spiService(service), argTransformer(argTransformer), userInputManager(userInputManager) {}

/*
Entry point for command
*/
void SpiController::handleCommand(const TerminalCommand& cmd) {

    if (cmd.getRoot() == "sniff") {
        handleSniff();
    }

    else if (cmd.getRoot() == "flash") {
        if (cmd.getSubcommand() == "probe") {
            handleFlashProbe();
        } else if (cmd.getSubcommand() == "read") {
            handleFlashRead(cmd);
        } else if (cmd.getSubcommand() == "write") {
            handleFlashWrite();
        } else if (cmd.getSubcommand() == "erase") {
            handleFlashErase();
        } else {
            terminalView.println("Unknown SPI flash command. Use: probe, read, write, erase");
        }
    } 

    else if (cmd.getRoot() == "config") {
        handleConfig();
    }
    
    else {
        terminalView.println("");
        terminalView.println("Unknown SPI command. Usage:");
        terminalView.println("  sniff");
        terminalView.println("  flash probe");
        terminalView.println("  flash read <addr> <len>");
        terminalView.println("  flash write");
        terminalView.println("  flash erase");
        terminalView.println("  config");
        terminalView.println("  [..]");
        terminalView.println("");
    }
}

/*
Entry point for instructions
*/
void SpiController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("SPI Instruction [NYI]");
}

/*
Sniff
*/
void SpiController::handleSniff() {
        terminalView.println("SPI sniff [NYI]");
}

/*
Flash Probe
*/
void SpiController::handleFlashProbe() {
    uint8_t id[3] = {0};
    spiService.readFlashIdRaw(id);

    std::stringstream idStr;
    terminalView.println("");
    idStr << "SPI Flash ID: "
          << std::hex << std::uppercase << std::setfill('0')
          << std::setw(2) << (int)id[0] << " "
          << std::setw(2) << (int)id[1] << " "
          << std::setw(2) << (int)id[2];
    terminalView.println(idStr.str());

    // Check for common invalid responses
    if ((id[0] == 0x00 && id[1] == 0x00 && id[2] == 0x00) ||
        (id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF)) {    
        terminalView.println("No SPI flash detected (bus error or missing chip).");
        return;
    }

    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);

    // Known in database
    if (chip) {
        terminalView.println("Manufacturer: " + std::string(chip->manufacturerName));
        terminalView.println("Model: " + std::string(chip->modelName));
        terminalView.println("Capacity: " +
            std::to_string(chip->capacityBytes / (1024UL * 1024UL)) + " MB\n");
        return;
    }

    // Fallback, not a known chip
    const char* manufacturer = findManufacturerName(id[0]);
    terminalView.println("Manufacturer: " + std::string(manufacturer));

    // Estimate Capacity
    uint32_t size = 1UL << id[2];
    std::stringstream sizeStr;
    if (size >= (1024 * 1024)) {
        sizeStr << (size / (1024 * 1024)) << " MB (guessed)";
    } else {
        sizeStr << size << " bytes (guessed)";
    }
    terminalView.println("Estimated capacity: " + sizeStr.str());
    terminalView.println("");
}

/*
Flash Read
*/
void SpiController::handleFlashRead(const TerminalCommand& cmd) {
    // Args (addr, length)
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());
    
    if (args.size() < 2) {
        terminalView.println("Usage: flash read <address> <length>");
        return;
    }
    
    // Check valid number
    if (!argTransformer.isValidNumber(args[0]) || !argTransformer.isValidNumber(args[1])) {
        terminalView.println("Error: Invalid address or length. Use decimal or 0x-prefixed hex.\n");
        return;
    }
    
    // Parse
    uint32_t address = argTransformer.parseHexOrDec32(args[0]);
    uint32_t length = argTransformer.parseHexOrDec32(args[1]);
    if (length == 0) {
        terminalView.println("Error: Length must be greater than 0.\n");
        return;
    }
    
    // Detect flash
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    if ((id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF) ||
    (id[0] == 0x00 && id[1] == 0x00 && id[2] == 0x00)) {
        terminalView.println("No SPI flash detected (bus error or missing chip).\n");
        return;
    }

    
    // Read flash in chunks
    terminalView.println("SPI Flash Read: In progress... Press [ENTER] to stop");
    terminalView.println("");
    readFlashInChunks(address, length);
    terminalView.println("");
}

void SpiController::readFlashInChunks(uint32_t address, uint32_t length) {
    uint8_t buffer[1024];
    uint32_t remaining = length;
    uint32_t currentAddr = address;

    // Verify flash capacity
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashCapacity = 0;
    if (chip) {
        flashCapacity = chip->capacityBytes;
    } else {
        flashCapacity = spiService.calculateFlashCapacity(id[2]);
        std::stringstream capStr;
        capStr << "Estimated capacity from ID: " << (flashCapacity >> 20) << " MB";
        terminalView.println(capStr.str());
    }

    // Display chunks
    while (remaining > 0) {
        uint32_t chunkSize = (remaining > 1024) ? 1024 : remaining;
        spiService.readFlashData(currentAddr, buffer, chunkSize);

        for (uint32_t i = 0; i < chunkSize; i += 16) {
            std::stringstream line;

            // Address
            line << std::hex << std::uppercase << std::setfill('0')
                 << std::setw(6) << (currentAddr + i) << ": ";

            // Hex
            for (uint32_t j = 0; j < 16; ++j) {
                if (i + j < chunkSize) {
                    line << std::setw(2) << (int)buffer[i + j] << " ";
                } else {
                    line << "   ";
                }
            }

            // ASCII
            line << " ";
            for (uint32_t j = 0; j < 16; ++j) {
                if (i + j < chunkSize) {
                    char c = static_cast<char>(buffer[i + j]);
                    line << (isprint(c) ? c : '.');
                }
            }
            terminalView.println(line.str());

            // Check user ENTER to stop
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nRead interrupted by user.");
                return;
            }
        }

        currentAddr += chunkSize;
        remaining -= chunkSize;
    }
}

/*
Flash Write
*/
void SpiController::handleFlashWrite() {
    terminalView.println("SPI flash write [NYI]");
}

/*
Flash Erase
*/
void SpiController::handleFlashErase() {
    terminalView.println("SPI flash erase [NYI]");
}

/*
Config
*/
void SpiController::handleConfig() {
    terminalView.println("\nSPI Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t mosi = userInputManager.readValidatedPinNumber("MOSI pin", state.getSpiMOSIPin(), forbidden);
    state.setSpiMOSIPin(mosi);

    uint8_t miso = userInputManager.readValidatedPinNumber("MISO pin", state.getSpiMISOPin(), forbidden);
    state.setSpiMISOPin(miso);

    uint8_t sclk = userInputManager.readValidatedPinNumber("SCLK pin", state.getSpiCLKPin(), forbidden);
    state.setSpiCLKPin(sclk);

    uint8_t cs = userInputManager.readValidatedPinNumber("CS pin", state.getSpiCSPin(), forbidden);
    state.setSpiCSPin(cs);

    uint32_t freq = userInputManager.readValidatedUint32("Frequency", state.getSpiFrequency());
    state.setSpiFrequency(freq);

    spiService.configure(mosi, miso, sclk, cs, freq);

    terminalView.println("SPI configured.\n");
}

void SpiController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
