#include "SpiController.h"

SpiController::SpiController(ITerminalView& terminalView, IInput& terminalInput, 
                             SpiService& spiService, SdService& sdService, ArgTransformer& argTransformer,
                             UserInputManager& userInputManager, BinaryAnalyzeManager& binaryAnalyzeManager)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      spiService(spiService),
      sdService(sdService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      binaryAnalyzeManager(binaryAnalyzeManager)
{}

/*
Entry point for command
*/
void SpiController::handleCommand(const TerminalCommand& cmd) {

    if (cmd.getRoot() == "sniff") {
        handleSniff();
    }

    else if(cmd.getRoot() == "sdcard") {
        handleSdCard();
    }

    else if(cmd.getRoot() == "slave") {
        handleSlave();
    }

    else if (cmd.getRoot() == "flash") {
        if (cmd.getSubcommand() == "probe") {
            handleFlashProbe();
        } else if (cmd.getSubcommand() == "analyze") {
            handleFlashAnalyze(cmd);
        } else if (cmd.getSubcommand() == "strings") {
            handleFlashStrings(cmd);
        } else if (cmd.getSubcommand() == "search") {
            handleFlashSearch(cmd);
        } else if (cmd.getSubcommand() == "read") {
            handleFlashRead(cmd);
        } else if (cmd.getSubcommand() == "write") {
            handleFlashWrite(cmd);
        } else if (cmd.getSubcommand() == "erase") {
            handleFlashErase(cmd);
        } else {
            terminalView.println("Unknown SPI flash command. Use: probe, analyze, strings, read, write, erase");
        }
    } 

    else if (cmd.getRoot() == "config") {
        handleConfig();
    }
    
    else {
        handleHelp();
    }
}

/*
Entry point for instructions
*/
void SpiController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = spiService.executeByteCode(bytecodes);
    if (!result.empty()) {
        terminalView.println("SPI Read:\n");
        terminalView.println(result);
    }
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
Flash Analyze
*/
void SpiController::handleFlashAnalyze(const TerminalCommand& cmd) {
    if (!checkFlashPresent()) return;

    // Start address if any
    uint32_t start = 0;
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());
    if (!args.empty()) start = argTransformer.parseHexOrDec32(args[0]);

    terminalView.println("\nSPI Flash Analyze: SPI Flash from 0x" + argTransformer.toHex(start, 6) + "... Press [ENTER] to stop.");

    // Get flash size
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashSize = chip ? chip->capacityBytes : spiService.calculateFlashCapacity(id[2]);

    // Analyze
    BinaryAnalyzeManager::AnalysisResult result = binaryAnalyzeManager.analyze(start, flashSize);

    // Calculate Summary
    float printablePct = 100.0f * result.printableTotal / result.totalBytes;
    float nullsPct     = 100.0f * result.nullsTotal     / result.totalBytes;
    float ffPct        = 100.0f * result.ffTotal        / result.totalBytes;
    uint32_t dataBytes = result.totalBytes - (result.nullsTotal + result.ffTotal);
    float dataPct      = 100.0f * dataBytes / result.totalBytes;
    float normalizedEntropy = result.avgEntropy / 8.0f;
    std::string bar = "[";
    int barLength = 20;
    int filled = std::round(normalizedEntropy * barLength);
    for (int i = 0; i < barLength; ++i)
        bar += (i < filled) ? '#' : '.';
    bar += "]";

    // Entropy infos
    std::string interpretation;
    if (normalizedEntropy >= 0.95f)
        interpretation = "→ likely encrypted/compressed";
    else if (normalizedEntropy >= 0.85f)
        interpretation = "→ mostly compressed";
    else if (normalizedEntropy >= 0.65f)
        interpretation = "→ mixed content";
    else if (normalizedEntropy >= 0.4f)
        interpretation = "→ partially structured";
    else if (normalizedEntropy >= 0.2f)
        interpretation = "→ contains padding";
    else
        interpretation = "→ likely empty";

    // Display Summary
    terminalView.println("");
    terminalView.println("\nSPI Flash Analysis Summary:");
    terminalView.println("  Blocks analyzed     : " + std::to_string(result.blocks));
    terminalView.println("  Total bytes         : " + std::to_string(result.totalBytes));
    terminalView.println("  Avg entropy (0-1)   : " + argTransformer.formatFloat(normalizedEntropy, 2) + " " + bar + " " + interpretation);
    terminalView.println("  Printable ASCII     : " + std::to_string(result.printableTotal) + " (" + argTransformer.formatFloat(printablePct, 1) + "%)");
    terminalView.println("  Null bytes (0x00)   : " + std::to_string(result.nullsTotal)     + " (" + argTransformer.formatFloat(nullsPct, 1) + "%)");
    terminalView.println("  FF bytes (0xFF)     : " + std::to_string(result.ffTotal)         + " (" + argTransformer.formatFloat(ffPct, 1) + "%)");
    terminalView.println("  Likely useful data  : " + std::to_string(dataBytes)              + " (" + argTransformer.formatFloat(dataPct, 1) + "%)");

    // Secrets
    if (!result.foundSecrets.empty()) {
        terminalView.println("\n  Detected sensitive patterns:");
        for (const auto& entry : result.foundSecrets) {
            terminalView.println("    " + entry);
        }
    }

    // Files
    if (!result.foundFiles.empty()) {
        terminalView.println("\n  Detected file signatures:");
        for (const auto& entry : result.foundFiles) {
            terminalView.println("    " + entry);
        }
    } else {
        terminalView.println("\n  No known file signatures found.");
    }

    terminalView.println("\n  SPI Flash Analyze: Done.\n");
}

/*
Flash Strings
*/
void SpiController::handleFlashStrings(const TerminalCommand& cmd) {
    // Check chip presence
    if (!checkFlashPresent()) return;

    terminalView.println("\nSPI Flash: Extracting strings... Press [ENTER] to stop.\n");

    // Validate and parse args
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());
    uint8_t minStringLen = 7; // default
    if (!args.empty() && argTransformer.isValidNumber(args[0])) {
        minStringLen = argTransformer.parseHexOrDec(args[0]);
        if (minStringLen < 1) minStringLen = 1;
        if (minStringLen > 255) minStringLen = 255;
    }

    const uint32_t blockSize = 512;
    uint8_t buffer[blockSize];
    std::string currentStr;
    uint32_t currentAddr = 0;
    uint32_t stringStartAddr = 0;
    bool inString = false;

    // Get flash size
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashSize = chip ? chip->capacityBytes : spiService.calculateFlashCapacity(id[2]);

    // Read flash in chuncks
    for (uint32_t addr = 0; addr < flashSize; addr += blockSize) {
        spiService.readFlashData(addr, buffer, blockSize);

        // Read blocks
        for (uint32_t i = 0; i < blockSize; ++i) {
            uint8_t b = buffer[i];
            uint32_t absoluteAddr = addr + i;

            if (b >= 32 && b <= 126) {  // printable ASCII
                if (!inString) {
                    inString = true;
                    stringStartAddr = absoluteAddr;
                }
                currentStr += static_cast<char>(b);
            } else {
                if (inString && currentStr.length() >= minStringLen) {
                    terminalView.println(
                        "0x" + argTransformer.toHex(stringStartAddr, 6) + ": " + currentStr
                    );
                }
                currentStr.clear();
                inString = false;
            }

            // Quit if user presses ENTER
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nSPI Flash: Extraction cancelled by user.");
                return;
            }
        }
    }

    // if remaining string
    if (inString && currentStr.length() >= minStringLen) {
        terminalView.println(
            "0x" + argTransformer.toHex(stringStartAddr, 6) + ": " + currentStr
        );
    }

    terminalView.println("\nSPI Flash: String extraction complete.\n");
}

/*
Flash Search
*/
void SpiController::handleFlashSearch(const TerminalCommand& cmd) {
    // Check chip presence
    if (!checkFlashPresent()) return;

    // Checks args
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());
    if (args.empty() || args[0].empty()) {
        terminalView.println("Usage: flash search <string> [start_address]");
        return;
    }

    // Parse
    std::string needle = args[0];
    std::vector<uint8_t> pattern(needle.begin(), needle.end());
    uint32_t startAddr = 0;
    if (args.size() >= 2 && argTransformer.isValidNumber(args[1])) {
        startAddr = argTransformer.parseHexOrDec32(args[1]);
    }

    terminalView.println("\nSearching for \"" + needle + "\" in SPI flash from 0x" + argTransformer.toHex(startAddr, 6) + "... Press [ENTER] to stop.\n");

    const uint32_t blockSize = 512;
    const uint32_t contextSize = 16;  // characters before and after
    uint8_t buffer[blockSize + 32];

    // Get flash size
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashSize = chip ? chip->capacityBytes : spiService.calculateFlashCapacity(id[2]);

    // Read flash in chunks
    for (uint32_t addr = startAddr; addr < flashSize; addr += blockSize - pattern.size()) {
        spiService.readFlashData(addr, buffer, blockSize + pattern.size() - 1);
        
        // Read block
        for (uint32_t i = 0; i <= blockSize; ++i) {
            if (i + pattern.size() > blockSize + pattern.size() - 1) break;

            bool match = true;
            for (size_t j = 0; j < pattern.size(); ++j) {
                if (buffer[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            
            // Found a matching string
            if (match) {
                uint32_t matchAddr = addr + i;
                std::string context;

                // Before the pattern
                for (int j = (int)i - (int)contextSize; j < (int)i; ++j) {
                    if (j >= 0) {
                        char c = (char)buffer[j];
                        context += (isprint(c) ? c : '.');
                    }
                }

                // Pattern
                context += "[";
                for (size_t j = 0; j < pattern.size(); ++j) {
                    char c = (char)buffer[i + j];
                    context += (isprint(c) ? c : '.');
                }
                context += "]";

                // After the pattern
                for (uint32_t j = i + pattern.size(); j < i + pattern.size() + contextSize && j < blockSize + pattern.size(); ++j) {
                    char c = (char)buffer[j];
                    context += (isprint(c) ? c : '.');
                }

                terminalView.println("0x" + argTransformer.toHex(matchAddr, 6) + ": " + context);
            }

            // Allow user to interrupt
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nSPI Flash Search: Cancelled by user.\n");
                return;
            }
        }
    }

    terminalView.println("\nSearch complete.");
}

/*
Flash Read
*/
void SpiController::handleFlashRead(const TerminalCommand& cmd) {
    // Check chip presence
    if (!checkFlashPresent()) return;
    
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
    
    // Read flash in chunks
    terminalView.println("SPI Flash Read: In progress... Press [ENTER] to stop");
    terminalView.println("");
    readFlashInChunks(address, length);
    terminalView.println("");
}

/*
Flash Read In Chunks
*/
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
void SpiController::handleFlashWrite(const TerminalCommand& cmd) {
    // Check chip presence
    if (!checkFlashPresent()) return;

    // Confirm
    if (!userInputManager.readYesNo("SPI Flash Write: Confirm write operation?", false)) {
        terminalView.println("SPI Flash Write: Cancelled.\n");
        return;
    }

    // Split args
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());
    if (args.size() < 2) {
        terminalView.println("Usage: flash write <address> <data>");
        return;
    }
    
    // Validate addr
    if (!argTransformer.isValidNumber(args[0])) {
        terminalView.println("Invalid address format.");
        return;
    }

    // Parse addr
    uint32_t address = argTransformer.parseHexOrDec32(args[0]);
    std::vector<uint8_t> data;

    // Parse data, ascii string
    if (args[1].front() == '"') {
        std::string joined = cmd.getArgs();
        size_t start = joined.find('"') + 1;
        size_t end = joined.find('"', start);
        if (end == std::string::npos) {
            terminalView.println("Missing closing quote for ASCII string.");
            return;
        }
        std::string raw = joined.substr(start, end - start);
        std::string decoded = argTransformer.decodeEscapes(raw);
        data.assign(decoded.begin(), decoded.end());
    // Dec or Hex
    } else {
        for (size_t i = 1; i < args.size(); ++i) {
            if (!argTransformer.isValidNumber(args[i])) continue;
            data.push_back(argTransformer.parseHexOrDec(args[i]));
        }
    }

    // Validate data
    if (data.empty()) {
        terminalView.println("SPI Flash Write: Invalid Data format.");
        return;
    }

    // Write data
    terminalView.println("Writing " + std::to_string(data.size()) + " byte(s) at address 0x" + 
                         argTransformer.toHex(address, 6));

    uint32_t freq = state.getSpiFrequency();
    spiService.writeFlashPatch(address, data, freq); // patch the entire sector

    terminalView.println("SPI Flash Write: Complete.\n");
}

/*
Flash Erase
*/
void SpiController::handleFlashErase(const TerminalCommand& cmd) {
    // Check chip presence
    if (!checkFlashPresent()) return;
    
    terminalView.println("");
    if (!userInputManager.readYesNo("SPI Flash Erase: Erase entire flash memory?", false)) {
        terminalView.println("SPI Flash Erase: Cancelled.\n");
        return;
    }

    // Get infos about the chip
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t freq = state.getSpiFrequency();
    const uint32_t sectorSize = 4096; // standard
    uint32_t flashSize = 0;
    
    // Known
    if (chip) {
        flashSize = chip->capacityBytes;
        terminalView.println("Flash: " + std::string(chip->modelName));
    // Unknown
    } else {
        flashSize = spiService.calculateFlashCapacity(id[2]);
        terminalView.println("Erasing unknown flash chip.");
    }
    terminalView.println("Capacity: " + std::to_string(flashSize >> 20) + " MB");

    // Erase sectors and display progression
    const uint32_t totalSectors = flashSize / sectorSize;
    terminalView.print("In progress");
    for (uint32_t i = 0; i < totalSectors; ++i) {
        uint32_t addr = i * sectorSize;
        spiService.eraseSector(addr, freq);

        // Display a dot
        if (i % 64 == 0) terminalView.print(".");
    }

    terminalView.println("\r\nSPI Flash Erase: Complete.\n");
}

/*
Check Chip
*/
bool SpiController::checkFlashPresent() {
    uint8_t id[3];
    spiService.readFlashIdRaw(id);

    bool invalid = (id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF) ||
                   (id[0] == 0x00 && id[1] == 0x00 && id[2] == 0x00);

    if (invalid) {
        terminalView.println("No SPI flash detected (bus error or missing chip).\n");
        return false;
    }

    return true;
}

/*
Slave
*/
void SpiController::handleSlave() {
#ifdef DEVICE_M5STICK
    terminalView.println("SPI Slave: Not supported on M5Stick devices due to shared SPI bus.");
    return;
#endif
    spiService.end(); // Stop master mode if active
    
    int sclk = state.getSpiCLKPin();
    int miso = state.getSpiMISOPin();
    int mosi = state.getSpiMOSIPin();
    int cs   = state.getSpiCSPin();

    terminalView.println("SPI Slave: In progress... Press [ENTER] to stop.");
    spiService.startSlave(sclk, miso, mosi, cs);

    terminalView.println("");
    terminalView.println("  [INFO]");
    terminalView.println("    SPI Slave mode listens passively on the SPI bus.");
    terminalView.println("    Any command sent by a SPI master will be captured and logged");
    terminalView.println("    Data is only captured when CS (chip select) is active.");
    terminalView.println("");

    while (true) {
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        // Read slave data from master
        auto packets = spiService.getSlaveData();
        for (const auto& packet : packets) {
            std::stringstream ss;
            ss << "[MOSI] ";
            for (uint8_t b : packet) {
                ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)b << " ";
            }
            terminalView.println(ss.str());
        }
        delay(1);
    }

    spiService.stopSlave(sclk, miso, mosi, cs);
    spiService.configure(mosi, miso, sclk, cs, state.getSpiFrequency()); // Reconfigure master
    terminalView.println("SPI Slave: Cancelled by user.");
}

/*
SD Card
*/
void SpiController::handleSdCard() {

    terminalView.println("SD Card: Mounting...");
    delay(500); // let the user see this ^

    bool success = sdService.configure(
            state.getSpiCLKPin(), 
            state.getSpiMISOPin(),
            state.getSpiMOSIPin(), 
            state.getSpiCSPin()
    );

    if (!success) {
        terminalView.println("SD Card: Mount failed. Check config and wiring and try again.\n");
    } else {
        terminalView.println("SD Card: Mounted successfully. Loading...\n");
    
        // Root content
        auto elements = sdService.listElements("/");
        if (elements.empty()) {
            terminalView.println("[Root is empty]");
        } else {
            terminalView.println("Root content:");
            for (const auto& item : elements) {
                terminalView.println("  - " + item);
            }
        }
    
        terminalView.println("");
        terminalView.println("  [INFO] SD card interface is not yet fully implemented.");
        terminalView.println("         You can use the USB Stick mode to mount the card");
        terminalView.println("         as a USB drive and access it from your computer.\n");
    }

    // Close and reconfigure
    sdService.close();
    spiService.end();
    ensureConfigured();
}

/*
Help
*/
void SpiController::handleHelp() {
    terminalView.println("");
    terminalView.println("Unknown SPI command. Usage:");
    terminalView.println("  sniff");
    terminalView.println("  sdcard");
    terminalView.println("  slave");
    terminalView.println("  flash probe");
    terminalView.println("  flash analyze [start address]");
    terminalView.println("  flash strings [length]");
    terminalView.println("  flash search <text>");
    terminalView.println("  flash read <addr> <len>");
    terminalView.println("  flash write <addr> <data>");
    terminalView.println("  flash erase");
    terminalView.println("  config");
    terminalView.println("  raw instructions, e.g: [0x9F r:3]");
    terminalView.println("");
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

    // Reconfigure, user could have used these pins for another mode
    uint8_t sclk = state.getSpiCLKPin();
    uint8_t miso = state.getSpiMISOPin();
    uint8_t mosi = state.getSpiMOSIPin();
    uint8_t cs   = state.getSpiCSPin();
    int freq   = state.getSpiFrequency();
    spiService.configure(mosi, miso, sclk, cs, freq);
}
