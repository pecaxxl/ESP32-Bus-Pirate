#include "I2cEepromShell.h"

I2cEepromShell::I2cEepromShell(
    ITerminalView& view,
    IInput& input,
    I2cService& i2cService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    BinaryAnalyzeManager& binaryAnalyzeManager
) : terminalView(view),
    terminalInput(input),
    i2cService(i2cService),
    argTransformer(argTransformer),
    userInputManager(userInputManager),
    binaryAnalyzeManager(binaryAnalyzeManager) {}

void I2cEepromShell::run(uint8_t addr) {
    terminalView.println("I2C EEPROM Shell started...\n");
    
    // Select EEPROM model
    int selectedModelIndex = userInputManager.readValidatedChoiceIndex("Select EEPROM type", models, 0);
    if (selectedModelIndex < 0) {
        terminalView.println("Invalid selection. Aborting.\n");
        return;
    }
    
    // Initialize EEPROM
    uint16_t selectedType = memoryLengths[selectedModelIndex];
    if (!i2cService.initEeprom(selectedType, selectedI2cAddress)) {
        terminalView.println("❌ EEPROM not detected at selected configuration.");
        return;
    }
    
    // Set variables
    terminalView.println("✅ EEPROM initialized: " + models[selectedModelIndex] + "\n");
    selectedModel = models[selectedModelIndex];
    selectedLength = memoryLengths[selectedModelIndex];
    selectedI2cAddress = addr;
    initialized = true;

    while (true) {
        // Select action
        int index = userInputManager.readValidatedChoiceIndex("Select an EEPROM action", actions, 0);
        if (index == -1 || actions[index] == "🚪 Exit Shell") {
            terminalView.println("Exiting EEPROM shell...\n");
            break;
        }

        // Run selected action
        switch (index) {
            case 0: cmdProbe(); break;
            case 1: cmdAnalyze(); break;
            case 2: cmdRead(); break;
            case 3: cmdWrite(); break;
            case 4: cmdDump(); break;
            case 5: cmdErase(); break;
        }
    }
}

void I2cEepromShell::cmdProbe() {
    uint32_t length = i2cService.eepromLength();
    uint32_t memSize = i2cService.eepromGetMemorySize();
    uint16_t pageSize = i2cService.eepromPageSize();
    uint8_t addrBytes = i2cService.eepromAddressBytes();
    uint8_t writeTime = i2cService.eepromWriteTimeMs();

    terminalView.println("\n📄 EEPROM Summary:");
    terminalView.println(" • Capacity:     " + std::to_string(length) + " bytes");
    terminalView.println(" • Memory Size:  " + std::to_string(memSize) + " bytes");
    terminalView.println(" • Page Size:    " + std::to_string(pageSize) + " bytes");
    terminalView.println(" • Address Size: " + std::to_string(addrBytes) + " byte(s)");
    terminalView.println(" • Write Time:   " + std::to_string(writeTime) + " ms\n");
}

void I2cEepromShell::cmdAnalyze() {
    uint32_t eepromSize = i2cService.eepromLength();
    uint32_t start = 0;
    terminalView.println("🔍 Analyzing EEPROM content...\n");

    // Analyze the EEPROM content in chuncks
    BinaryAnalyzeManager::AnalysisResult result = binaryAnalyzeManager.analyze(
        start,
        eepromSize,
        [&](uint32_t addr, uint8_t* buf, uint32_t len) {
            for (uint32_t i = 0; i < len; ++i)
                buf[i] = i2cService.eepromReadByte(addr + i);
        }
    );

    // Format and print the analysis result
    auto summary = binaryAnalyzeManager.formatAnalysis(result);
    terminalView.println(summary);

    // Files
    if (!result.foundFiles.empty()) {
        terminalView.println("\n📁 Detected file signatures:");
        for (const auto& file : result.foundFiles) {
            terminalView.println("   - " + file);
        }
    }

    // Secrets
    if (!result.foundSecrets.empty()) {
        terminalView.println("\n🔑 Potential secrets found:");
        for (const auto& secret : result.foundSecrets) {
            terminalView.println("   - " + secret);
        }
    }

}

void I2cEepromShell::cmdRead() {
    auto addrStr = userInputManager.readValidatedHexString("Start address (e.g., 00FF00) ", 0, true);
    auto addr = argTransformer.parseHexOrDec16("0x" + addrStr);
    uint32_t eepromSize = i2cService.eepromLength();
    if (addr >= eepromSize) {
        terminalView.println("❌ Error: Start address is beyond EEPROM size.\n");
        return;
    }
    
    uint8_t count = userInputManager.readValidatedUint8("Number of bytes to read:", 16);
    terminalView.println("");
    if (addr + count > eepromSize) {
        count = eepromSize - addr;
    }

    const uint8_t bytesPerLine = 16;
    for (uint16_t i = 0; i < count; i += bytesPerLine) {
        std::vector<uint8_t> line;
        for (uint8_t j = 0; j < bytesPerLine && (i + j) < count; ++j) {
            line.push_back(i2cService.eepromReadByte(addr + i + j));
        }

        std::string formattedLine = argTransformer.toAsciiLine(addr + i, line);
        terminalView.println(formattedLine);
    }
    
    terminalView.println("");
}

void I2cEepromShell::cmdWrite() {
    auto addrStr = userInputManager.readValidatedHexString("Start address:", 0, true);
    auto addr = argTransformer.parseHexOrDec16("0x" + addrStr);
    auto hexStr = userInputManager.readValidatedHexString("Enter byte values (e.g., 01 A5 FF...) ", 0, true);
    auto data = argTransformer.parseHexList(hexStr);

    bool ok = true;
    for (size_t i = 0; i < data.size(); ++i) {
        i2cService.eepromWriteByte(addr + i, data[i]);
    }

    terminalView.println("✅ Data written.");
}

void I2cEepromShell::cmdDump() {
    uint32_t addr = 0;
    uint32_t count = i2cService.eepromLength();

    const uint8_t bytesPerLine = 16;
    for (uint32_t i = 0; i < count; i += bytesPerLine) {
        std::vector<uint8_t> line;
        for (uint8_t j = 0; j < bytesPerLine && (i + j) < count; ++j) {
            line.push_back(i2cService.eepromReadByte(addr + i + j));
        }

        std::string formatted = argTransformer.toAsciiLine(addr + i, line);
        terminalView.println(formatted);
    }

    terminalView.println("");
}

void I2cEepromShell::cmdErase() {
    bool confirm = userInputManager.readYesNo("⚠️  Are you sure you want to erase the EEPROM?", false);
    if (confirm) {
        terminalView.println("Erasing...");
        i2cService.eepromErase(0x00);
        terminalView.println("✅ EEPROM erased.");
    } else {
        terminalView.println("Operation cancelled.");
    }
}