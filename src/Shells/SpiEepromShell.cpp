#include "SpiEepromShell.h"

SpiEepromShell::SpiEepromShell(
    SpiService& spiService,
    ITerminalView& view,
    IInput& input,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager
) :
    spiService(spiService),
    terminalView(view),
    terminalInput(input),
    argTransformer(argTransformer),
    userInputManager(userInputManager)
{
}

void SpiEepromShell::run() {
    // Select EEPROM model
    int selectedModelIndex = userInputManager.readValidatedChoiceIndex("\nSelect EEPROM type", models, 0);
    if (selectedModelIndex < 0) {
        terminalView.println("Invalid selection. Aborting.\n");
        return;
    }

    // Get params
    eepromSize = memoryLengths[selectedModelIndex];
    pageSize = pageLengths[selectedModelIndex];
    auto mosi = state.getSpiMOSIPin();
    auto miso = state.getSpiMISOPin();
    auto sclk = state.getSpiCLKPin();
    auto cs = state.getSpiCSPin();
    
    // Initialize EEPROM
    bool ok = spiService.initEeprom(mosi, miso, sclk, cs, pageSize, eepromSize);
    if (!ok) {
        terminalView.println("Failed to initialize EEPROM. Please check connections.");
        return;
    }

    while (true) {
        terminalView.println("\n=== SPI EEPROM Shell ===");

        // Select action
        int index = userInputManager.readValidatedChoiceIndex("Select EEPROM action", actions, 0);

        // Quit
        if (index == -1 || actions[index] == "🚪 Exit Shell") {
            terminalView.println("Exiting SPI EEPROM Shell...\n");
            break;
        }
        // Run selected action
        switch (index) {
            case 0: cmdProbe(); break;
            case 1: cmdRead();  break;
            case 2: cmdWrite(); break;
            case 3: cmdDump();  break; 
            case 4: cmdErase(); break; 
            default:
                terminalView.println("Unknown action.");
                break;
        }
    }
    spiService.closeEeprom();
}

void SpiEepromShell::cmdProbe() {
    terminalView.println("\n[NYI] 🔍 SPI EEPROM Probe...");
}

void SpiEepromShell::cmdRead() {
    terminalView.println("\n📖 Read EEPROM");

    auto addrStr = userInputManager.readValidatedHexString("Start address (e.g., 00FF00) ", 0, true);
    uint32_t addr = argTransformer.parseHexOrDec32("0x" + addrStr);

    if (addr >= eepromSize) {
        terminalView.println("\n ❌ Error: Start address is beyond EEPROM size.\n");
        return;
    }

    uint32_t count = userInputManager.readValidatedUint32("Number of bytes to read:", 16);
    if (addr + count > eepromSize) {
        count = eepromSize - addr;
    }

    terminalView.println("");
    const uint8_t bytesPerLine = 16;

    for (uint32_t i = 0; i < count; i += bytesPerLine) {
        uint8_t buffer[bytesPerLine];
        uint8_t lenToRead = std::min<uint32_t>(bytesPerLine, count - i);

        bool ok = spiService.readEepromBuffer(addr + i, buffer, lenToRead);
        if (!ok) {
            terminalView.println("\n ❌ Read failed at 0x" + argTransformer.toHex(addr + i, 6));
            return;
        }

        std::vector<uint8_t> line(buffer, buffer + lenToRead);
        std::string formattedLine = argTransformer.toAsciiLine(addr + i, line);
        terminalView.println(formattedLine);
    }

    terminalView.println("");
}


void SpiEepromShell::cmdWrite() {
    terminalView.println("\n✏️  Write EEPROM");

    uint32_t addr = userInputManager.readValidatedUint32("Start address:", 0);

    if (userInputManager.readYesNo("Write an ASCII string?", true)) {
        terminalView.print("Enter ASCII string: ");
        std::string input = userInputManager.getLine();
        std::string decoded = argTransformer.decodeEscapes(input);
        bool ok = spiService.writeEepromBuffer(addr, (const uint8_t*)decoded.data(), decoded.size());
        terminalView.println(ok ? "\n ✅ Write OK" : "\n ❌ Write failed");
    } else {
        std::string hexStr = userInputManager.readValidatedHexString("Enter hex bytes (e.g., AA BB CC) ", 0, true);
        std::vector<uint8_t> data = argTransformer.parseHexList(hexStr);
        bool ok = spiService.writeEepromBuffer(addr, data.data(), data.size());
        terminalView.println(ok ? "\n ✅ Write OK" : "\n ❌ Write failed");
    }
}

void SpiEepromShell::cmdDump() {
    terminalView.println("\n🗃️ EEPROM Dump: Reading entire memory...");

    const uint32_t totalSize = eepromSize;
    const uint32_t lineSize = 16;
    uint8_t buffer[lineSize];

    for (uint32_t addr = 0; addr < totalSize; addr += lineSize) {
        // Read line
        bool ok = spiService.readEepromBuffer(addr, buffer, lineSize);
        if (!ok) {
            terminalView.println("\n ❌ Read failed at 0x" + argTransformer.toHex(addr, 6));
            return;
        }

        // Format and print line
        std::vector<uint8_t> line(buffer, buffer + lineSize);
        std::string formattedLine = argTransformer.toAsciiLine(addr, line);
        terminalView.println(formattedLine);

        // Quit
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("\n ❌ Dump cancelled by user.");
            return;
        }
    }

    terminalView.println("\n ✅ EEPROM Dump Done.");
}

void SpiEepromShell::cmdErase() {
    terminalView.println("\n💣 EEPROM Erase: Writing 0x00 to entire memory...");

    if (!userInputManager.readYesNo("Confirm erase?", false)) {
        terminalView.println("Erase cancelled.");
        return;
    }

    const uint32_t totalSize = eepromSize;
    const uint32_t blockSize = 64;
    uint8_t zeros[blockSize] = {0};

    for (uint32_t addr = 0; addr < totalSize; addr += blockSize) {
        bool ok = spiService.writeEepromBuffer(addr, zeros, blockSize);
        if (!ok) {
            terminalView.println("\n ❌ Write failed at 0x" + argTransformer.toHex(addr, 6));
            return;
        }

        // Progression feedback
        if (addr % 512 == 0) terminalView.print(".");
    }

    terminalView.println("\n ✅ EEPROM Erase Done.");
}
