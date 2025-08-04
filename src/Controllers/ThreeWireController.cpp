#include "ThreeWireController.h"

/*
Constructor
*/
ThreeWireController::ThreeWireController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    UserInputManager& userInputManager,
    ThreeWireService& threeWireService,
    ArgTransformer& argTransformer)
  : terminalView(terminalView),
    terminalInput(terminalInput),
    userInputManager(userInputManager),
    threeWireService(threeWireService),
    argTransformer(argTransformer) {}

/*
Entry point for command
*/
void ThreeWireController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "config")       handleConfig();
    else if (cmd.getRoot() == "eeprom")  handleEeprom(cmd);
    else                                 handleHelp();
}

/*
Entry point for instructions
*/
void ThreeWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("Instruction handling not yet implemented");
}

/*
EEPROM
*/
void ThreeWireController::handleEeprom(const TerminalCommand& cmd) {
    std::string sub = cmd.getSubcommand();

    if      (sub == "probe") handleEepromProbe();
    else if (sub == "read")  handleEepromRead(cmd);
    else if (sub == "write") handleEepromWrite(cmd);
    else if (sub == "dump")  handleEepromDump();
    else if (sub == "erase") handleEepromErase();
    else                     handleHelp();
}

/*
EEPROM Probe
*/
void ThreeWireController::handleEepromProbe() {
    bool isOrg8 = state.isThreeWireOrg8();
    bool isBlank = true;

    if (isOrg8) {
        std::vector<uint8_t> data = threeWireService.dump8();
        for (uint8_t val : data) {
            if (val != 0xFF) {
                isBlank = false;
                break;
            }
        }
    } else {
        std::vector<uint16_t> data = threeWireService.dump16();
        for (uint16_t val : data) {
            if (val != 0xFFFF) {
                isBlank = false;
                break;
            }
        }
    }

    if (!isBlank) {
        terminalView.println("3WIRE EEPROM: Detected ✅");
    } else {
        terminalView.println("3WIRE EEPROM: No EEPROM detected or EEPROM is blank ❌");
    }
}

/*
EEPROM Read
*/
void ThreeWireController::handleEepromRead(const TerminalCommand& cmd) {
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());
    
    if (args.empty()) {
        terminalView.println("Usage: eeprom read <addr> [count]");
        return;
    }

    if (!argTransformer.isValidNumber(args[0])) {
        terminalView.println("3WIRE EEPROM: Invalid address value. Must be a number.");
        return;
    }

    uint16_t addr = argTransformer.parseHexOrDec16(args[0]);
    uint16_t count = 1;
    bool isOrg8 = state.isThreeWireOrg8();

    if (args.size() == 2) {
        if (!argTransformer.isValidNumber(args[1])) {
            terminalView.println("Invalid count value. Must be a number.");
            return;
        }
        count = argTransformer.parseHexOrDec16(args[1]);
        if (count == 0) count = 1;
    }

    if (count == 1) {
        if (isOrg8) {
            uint8_t val = threeWireService.read8(addr);
            terminalView.println("3WIRE EEPROM: Read 0x" + argTransformer.toHex(addr, 4) +
                                 " = 0x" + argTransformer.toHex(val, 2));
        } else {
            uint16_t val = threeWireService.read16(addr);
            terminalView.println("3WIRE EEPROM: [READ] 0x" + argTransformer.toHex(addr, 4) +
                                 " = 0x" + argTransformer.toHex(val, 4));
        }
    } else {
        if (isOrg8) {
            std::vector<uint8_t> values;
            for (uint16_t i = 0; i < count; ++i) {
                values.push_back(threeWireService.read8(addr + i));
            }
            for (size_t i = 0; i < values.size(); i += 16) {
                uint32_t displayAddr = addr + i;
                size_t chunkSize = std::min<size_t>(16, values.size() - i);
                std::vector<uint8_t> chunk(values.begin() + i, values.begin() + i + chunkSize);
                terminalView.println(argTransformer.toAsciiLine(displayAddr, chunk));
            }
        } else {
            std::vector<uint16_t> values;
            for (uint16_t i = 0; i < count; ++i) {
                values.push_back(threeWireService.read16(addr + i));
            }
            for (size_t i = 0; i < values.size(); i += 8) {
                uint32_t displayAddr = (addr + i) * 2;
                size_t chunkSize = std::min<size_t>(8, values.size() - i);
                std::vector<uint16_t> chunk(values.begin() + i, values.begin() + i + chunkSize);
                terminalView.println(argTransformer.toAsciiLine(displayAddr, chunk));
            }
        }
    }
}

/*
EEPROM Write
*/
void ThreeWireController::handleEepromWrite(const TerminalCommand& cmd) {
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());

    if (args.size() < 1) {
        terminalView.println("Usage: eeprom write <addr> [value]");
        return;
    }

    int addr = argTransformer.parseHexOrDec16(args[0]);

    bool isOrg8 = state.isThreeWireOrg8();
    threeWireService.writeEnable();

    if (args.size() >= 2) {
        uint16_t value = argTransformer.parseHexOrDec16(args[1]);
        if (isOrg8) {
            threeWireService.write8(addr, static_cast<uint8_t>(value));
            terminalView.println("3WIRE EEPROM: Write 0x" + argTransformer.toHex(addr, 4) +
                                 " = 0x" + argTransformer.toHex(value, 2) + " ✅");
        } else {
            threeWireService.write16(addr, value);
            terminalView.println("3WIRE EEPROM: Write 0x" + argTransformer.toHex(addr, 4) +
                                 " = 0x" + argTransformer.toHex(value, 4) + " ✅");
        }

    } else {
        std::string hexStr = userInputManager.readValidatedHexString("Enter value(s) to write (e.g., AA 01 00 BC)", 0, true);
        std::vector<uint8_t> data = argTransformer.parseHexList(hexStr);

        for (size_t i = 0; i < data.size(); ++i) {
            if (isOrg8) {
                threeWireService.write8(addr + i, data[i]);
                terminalView.println("3WIRE EEPROM: Write 0x" + argTransformer.toHex(addr + i, 4) +
                                     " = 0x" + argTransformer.toHex(data[i], 2) + " ✅");
            } else {
                if (i + 1 >= data.size()) break; // Incomplet
                uint16_t val = (data[i] << 8) | data[i + 1];
                threeWireService.write16(addr + (i / 2), val);
                terminalView.println("3WIRE EEPROM: Write 0x" + argTransformer.toHex(addr + (i / 2), 4) +
                                     " = 0x" + argTransformer.toHex(val, 4) + " ✅");
                ++i; // Consomme 2 bytes
            }
        }
    }

    threeWireService.writeDisable();
}

/*
EEPROM Dump
*/
void ThreeWireController::handleEepromDump() {
    bool isOrg8 = state.isThreeWireOrg8();
    uint16_t start = 0;

    terminalView.println("");
    if (isOrg8) {
        auto data = threeWireService.dump8();
        for (size_t i = start; i < data.size(); i += 16) {
            uint32_t addr = i;
            size_t chunkSize = std::min<size_t>(16, data.size() - i);
            std::vector<uint8_t> chunk(data.begin() + i, data.begin() + i + chunkSize);
            terminalView.println(argTransformer.toAsciiLine(addr, chunk));
        }
    } else {
        auto data = threeWireService.dump16();
        for (size_t i = start; i < data.size(); i += 8) {
            uint32_t addr = i * 2;
            size_t chunkSize = std::min<size_t>(8, data.size() - i);
            std::vector<uint16_t> chunk(data.begin() + i, data.begin() + i + chunkSize);
            terminalView.println(argTransformer.toAsciiLine(addr, chunk));
        }
    }
    terminalView.println("");

}

/*
EEPROM Erase
*/
void ThreeWireController::handleEepromErase() {
    
    auto confirmation = userInputManager.readYesNo("Are you sure you want to erase the EEPROM?", false);
    if (!confirmation) {
        terminalView.println("3WIRE EEPROM: ❌ Erase cancelled.");
        return;
    }

    threeWireService.writeEnable();
    threeWireService.eraseAll();
    threeWireService.writeDisable();
    bool isOrg8 = state.isThreeWireOrg8();
    bool success = true;

    if (isOrg8) {
        auto data = threeWireService.dump8();
        for (uint8_t val : data) {
            if (val != 0xFF) {
                success = false;
                break;
            }
        }
    } else {
        auto data = threeWireService.dump16();
        for (uint16_t val : data) {
            if (val != 0xFFFF) {
                success = false;
                break;
            }
        }
    }

    if (success) {
        terminalView.println("3WIRE EEPROM: ✅ Successfully erased.");
    } else {
        terminalView.println("3WIRE EEPROM: ❌ Erase verification failed.");
    }
}

/*
Help
*/
void ThreeWireController::handleHelp() {
    terminalView.println("Unknown 3WIRE command. Usage:");
    terminalView.println("  eeprom probe");
    terminalView.println("  eeprom read <addr> [count]");
    terminalView.println("  eeprom write <addr> [value]");
    terminalView.println("  eeprom dump");
    terminalView.println("  eeprom erase");
    terminalView.println("  config");
}

/*
Config
*/
void ThreeWireController::handleConfig() {
    terminalView.println("\n3WIRE Configuration:");
    
    // Pins
    const auto& forbidden = state.getProtectedPins();
    uint8_t cs = userInputManager.readValidatedPinNumber("CS pin", state.getThreeWireCsPin(), forbidden);
    state.setThreeWireCsPin(cs);
    uint8_t sk = userInputManager.readValidatedPinNumber("SK pin", state.getThreeWireSkPin(), forbidden);
    state.setThreeWireSkPin(sk);
    uint8_t di = userInputManager.readValidatedPinNumber("DI pin", state.getThreeWireDiPin(), forbidden);
    state.setThreeWireDiPin(di);
    uint8_t doPin = userInputManager.readValidatedPinNumber("DO pin", state.getThreeWireDoPin(), forbidden);
    state.setThreeWireDoPin(doPin);

    // EEPROM Model
    std::vector<std::string> modelOptions = threeWireService.getSupportedModels();
    int modelIndex = userInputManager.readValidatedChoiceIndex("\nSelect EEPROM model", modelOptions, state.getThreeWireEepromModelIndex());
    int modelId = threeWireService.resolveModelId(modelOptions[modelIndex]);
    terminalView.println("Selected model: " + modelOptions[modelIndex] + " (ID: " + std::to_string(modelId) + ")");
    state.setThreeWireEepromModelIndex(modelIndex);

    // Organization
    terminalView.println("\n⚠️  ORG is a physical pin on the EEPROM chip.");
    terminalView.println("   Tie it to GND for 8-bit (x8) organization.");
    terminalView.println("   Tie it to VCC for 16-bit (x16) organization.");
    terminalView.println("   This applies to chips with configurable ORG pins (most of them).");
    terminalView.println("   Fixed organization chips:");
    terminalView.println("     • 93xx56A → always 8-bit");
    terminalView.println("     • 93xx56B → always 16-bit\n");
    bool org8 = userInputManager.readYesNo("EEPROM organization 8 bits ?", false);
    state.setThreeWireOrg8(org8);

    // Configure the service
    threeWireService.configure(cs, sk, di, doPin, modelId, org8);
    terminalView.println("3WIRE EEPROM configured.\n");
    configured = true;
}

/*
Ensure configured
*/
void ThreeWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }

    // Pins could have been used elsewhere, reconfigure the service
    auto cs = state.getThreeWireCsPin();
    auto sk = state.getThreeWireSkPin();
    auto di = state.getThreeWireDiPin();
    auto doPin = state.getThreeWireDoPin();
    auto modelId = state.getThreeWireEepromModelIndex(); 
    auto org8 = state.isThreeWireOrg8();
    threeWireService.configure(cs, sk, di, doPin, modelId, org8);
}