// OneWireController.cpp
#include "OneWireController.h"

/*
Constructor
*/
OneWireController::OneWireController(ITerminalView& terminalView, IInput& terminalInput, 
                                    OneWireService& service, ArgTransformer& argTransformer,
                                    UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), oneWireService(service), argTransformer(argTransformer), userInputManager(userInputManager) {
}

/*
Entry point for command
*/
void OneWireController::handleCommand(const TerminalCommand& command) {
    if (command.getRoot()      == "scan") {
        handleScan();
    } 

    else if (command.getRoot() == "ping") {
        handlePing();
    } 

    else if (command.getRoot() == "sniff") {
        handleSniff();
    } 

    else if (command.getRoot() == "read") {
        handleRead();
    }
    
    else if (command.getRoot() == "write") {
        handleWrite(command);
    }

    else if (command.getRoot() == "config") {
        handleConfig();
    }
     
    else {
        handleHelp();
    }
}

/*
Entry point for instructions
*/
void OneWireController::handleInstruction(std::vector<ByteCode>& bytecodes) {
    auto result = oneWireService.executeByteCode(bytecodes);
    if (!result.empty()) {
        terminalView.println("OneWire Read:\n");
        terminalView.println(result);
    }
}

/*
Scan
*/
void OneWireController::handleScan() {
    terminalView.println("OneWire Scan: in progress...");

    oneWireService.resetSearch();

    uint8_t rom[8];
    int deviceCount = 0;

    while (oneWireService.search(rom)) {
        std::ostringstream oss;
        oss << "Device " << (++deviceCount) << ": ";
        for (int i = 0; i < 8; ++i) {
            oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
                << static_cast<int>(rom[i]) << " ";
        }

        uint8_t crc = oneWireService.crc8(rom, 7);
        if (crc != rom[7]) {
            oss << "(CRC error)";
        }

        terminalView.println(oss.str());
    }

    if (deviceCount == 0) {
        terminalView.println("OneWire Scan: No devices found.");
    }
}

/*
Ping
*/
void OneWireController::handlePing() {
    bool devicePresent = oneWireService.reset();
    if (devicePresent) {
        terminalView.println("OneWire Ping: Device present.");
    } else {
        terminalView.println("OneWire Ping: No device found.");
    }
}

/*
Read
*/
void OneWireController::handleRead() {
    handleIdRead();
    handleScratchpadRead();
}

/*
ID Read
*/
void OneWireController::handleIdRead() {
    uint8_t buffer[8];

        terminalView.println("OneWire Read: in progress.");
    if (!oneWireService.reset()) {
        terminalView.println("OneWire Read: No device found.");
        return;
    }

    oneWireService.write(0x33);  // Read ROM
    oneWireService.readBytes(buffer, 8);

    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (int i = 0; i < 8; ++i) {
        oss << std::setw(2) << static_cast<int>(buffer[i]);
        if (i < 7) oss << " ";
    }

    terminalView.println("ROM ID: " + oss.str());

    uint8_t crc = oneWireService.crc8(buffer, 7);
    if (crc != buffer[7]) {
        terminalView.println("OneWire Read:: CRC error on ROM ID.");
    }
}

/*
Scratchpad Read
*/
void OneWireController::handleScratchpadRead() {
    uint8_t scratchpad[8];
    if (!oneWireService.reset()) {
        terminalView.println("OneWire Read: No device found.");
        return;
    }

    oneWireService.write(0xAA);  // Read Scratchpad
    oneWireService.readBytes(scratchpad, 8);

    std::ostringstream oss;
    oss << "Scratchpad: ";
    for (int i = 0; i < 8; ++i) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)scratchpad[i];
        if (i < 8) oss << " ";
    }
    terminalView.println(oss.str());

    // CRC (last byte)
    uint8_t crc = oneWireService.crc8(scratchpad, 8);
    if (crc != scratchpad[8]) {
        terminalView.println("CRC error on scratchpad.");
    }
}

/*
Write
*/
void OneWireController::handleWrite(const TerminalCommand& cmd) {
    std::string sub = cmd.getSubcommand();

    if (sub == "id") {
        std::vector<uint8_t> idBytes = argTransformer.parseByteList(cmd.getArgs());

        if (idBytes.size() != 8) {
            terminalView.println("OneWire Write: ID must be exactly 8 bytes.");
            return;
        }

        handleIdWrite(idBytes);
    }

    else if (sub == "sp") {
        std::vector<uint8_t> spBytes = argTransformer.parseByteList(cmd.getArgs());

        if (spBytes.size() != 8) {
            terminalView.println("OneWire Write: Scratchpad must be exactly 8 bytes.");
            return;
        }

        handleScratchpadWrite(spBytes);
    }

    else {
        terminalView.println("OneWire Write: Invalid syntax. Use:");
        terminalView.println("  write id <8 bytes>");
        terminalView.println("  write sp <8 bytes>");
    }
}

/*
ID Write
*/
void OneWireController::handleIdWrite(std::vector<uint8_t> idBytes) {
    if (!oneWireService.reset()) {
        terminalView.println("OneWire Write: No device found.");
        return;
    }

    oneWireService.writeRw1990(state.getOneWirePin(), idBytes.data(), idBytes.size());
    terminalView.println("OneWire Write: ID write completed.");
}

/*
Scratchpad Write
*/
void OneWireController::handleScratchpadWrite(std::vector<uint8_t> scratchpadBytes) {
    if (!oneWireService.reset()) {
        terminalView.println("OneWire Write: No device found.");
        return;
    }

    oneWireService.skip();
    oneWireService.write(0x0F); // Scratchpad write command
    delayMicroseconds(20);
    oneWireService.writeBytes(scratchpadBytes.data(), 8);
    oneWireService.reset();

    terminalView.println("OneWire Write: Scratchpad write completed.");
}

/*
Config
*/
void OneWireController::handleConfig() {
    terminalView.println("");
    terminalView.println("OneWire Configuration:");

    uint8_t currentPin = state.getOneWirePin();

    terminalView.print("Data pin [" + std::to_string(currentPin) + "]: ");
    std::string input = userInputManager.getLine();
    uint8_t pin = input.empty() ? currentPin : argTransformer.toUint8(input);
    
    state.setOneWirePin(pin);
    oneWireService.configure(pin);

    terminalView.println("OneWire configured.");
    terminalView.println("");
}


/*
Sniff
*/
void OneWireController::handleSniff() {
    terminalView.println("OneWire Sniff Mode: observing data line.");
    terminalView.println("Press ENTER to stop.\n");

    uint8_t pin = state.getOneWirePin();
    pinMode(pin, INPUT);

    uint8_t bitCount = 0;
    uint8_t currentByte = 0;
    int lastState = 0;
    unsigned long lastTransition = micros();

    while (true) {
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') break;

        int state = digitalRead(pin);
        if (state != lastState) {
            unsigned long now = micros();
            unsigned long duration = now - lastTransition;
            lastTransition = now;

            std::stringstream ss;
            ss << "Transition to " << state << " after " << duration << " us";
            terminalView.println(ss.str());

            lastState = state;
        }
    }
    terminalView.println("");
    terminalView.println("OneWire Sniff Mode: Exited");
}

/*
Help
*/
void OneWireController::handleHelp() {
    terminalView.println("Unknown 1Wire command. Usage:");
    terminalView.println("  scan");
    terminalView.println("  ping");
    terminalView.println("  sniff");
    terminalView.println("  read");
    terminalView.println("  write id <8 bytes>");
    terminalView.println("  write sp <8 bytes>");
    terminalView.println("  config");
    terminalView.println("  raw instructions, [0X33 r:8] ...");
}

void OneWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}