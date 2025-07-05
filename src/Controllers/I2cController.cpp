#include "I2cController.h"

I2cController::I2cController(ITerminalView& terminalView, IInput& terminalInput, I2cService& i2cService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), i2cService(i2cService), argTransformer(argTransformer) {}

/*
Entry point to handle I2C command
*/
void I2cController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "scan") {
        handleScan();
    }

    else if (cmd.getRoot() == "sniff") {
        handleSniff();
    }

    else if (cmd.getRoot() == "ping") {
        handlePing(cmd);
    }

    else if (cmd.getRoot() == "write") {
        handleWrite(cmd);
    }

    else if (cmd.getRoot() == "read") {
        handleRead(cmd);
    }

    else if (cmd.getRoot() == "config") {
        handleConfig();
    }

    else {
        handleHelp();
    }
}

/*
Entry point to handle I2C instruction
*/
void I2cController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = i2cService.executeByteCode(bytecodes);
    if (!result.empty()) {
        terminalView.println("I2C Read:\n");
        terminalView.println(result);
    }
}

void I2cController::handleScan() {
    terminalView.println("I2C Scan: Scanning I2C bus...");
    terminalView.println("");
    bool found = false;

    for (uint8_t addr = 1; addr < 127; ++addr) {
        i2cService.beginTransmission(addr);
        if (i2cService.endTransmission() == 0) {
            std::stringstream ss;
            ss << "Found device at 0x" << std::hex << std::uppercase << (int)addr;
            terminalView.println(ss.str());
            found = true;
        }
    }

    if (!found) {
        terminalView.println("I2C Scan: No I2C devices found.");
    }
    terminalView.println("");
}

/*
Sniff
*/
void I2cController::handleSniff() {
    terminalView.println("I2C Sniff Mode: capturing raw bytes.");
    terminalView.println("Press ENTER to stop.\n");
    terminalView.println("");

    uint8_t sclPin = state.getI2cSclPin();
    uint8_t sdaPin = state.getI2cSdaPin();
    uint32_t freq = state.getI2cFrequency();
    uint32_t delayUs = 500000 / freq;
    if (delayUs < 1) delayUs = 1;

    uint8_t bitCount = 0;
    uint8_t currentByte = 0;
    int lastSCL = digitalRead(sclPin);

    unsigned long lastCharCheck = millis();

    while (true) {

        lastCharCheck = millis();
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') break;

        int scl = digitalRead(sclPin);
        int sda = digitalRead(sdaPin);

        // Detect rising edge on SCL
        if (lastSCL == LOW && scl == HIGH) {
            currentByte <<= 1;
            if (sda) currentByte |= 1;
            bitCount++;

            if (bitCount == 8) {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << (int)currentByte << " ";
                terminalView.print(ss.str());

                bitCount = 0;
                currentByte = 0;
            }
        }

        lastSCL = scl;
        delayMicroseconds(delayUs);
    }

    terminalView.println("");
    terminalView.println("I2C Sniff: Exited.");
}

/*
Ping
*/
void I2cController::handlePing(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: ping <I2C address>");
        return;
    }

    const std::string& arg = cmd.getSubcommand();
    uint8_t address = 0;

    std::stringstream ss(arg);
    int temp = 0;

    // Detect hex prefix
    if (arg.rfind("0x", 0) == 0 || arg.rfind("0X", 0) == 0) {
        ss >> std::hex >> temp;
    } else {
        ss >> std::dec >> temp;
    }

    if (ss.fail() || temp < 0 || temp > 127) {
        terminalView.println("I2C Ping: Invalid address format. Use hex (e.g. 0x3C).");
        return;
    }

    address = static_cast<uint8_t>(temp);

    std::stringstream result;
    result << "Ping 0x" << std::hex << std::uppercase << (int)address << ": ";

    i2cService.beginTransmission(address);
    uint8_t i2cResult = i2cService.endTransmission();

    if (i2cResult == 0) {
        result << "I2C Ping: ACK received! Device is present.";
    } else {
        result << "I2C Ping: No response (NACK or error).";
    }

    terminalView.println(result.str());
}

/*
Write
*/
void I2cController::handleWrite(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (args.size() < 2) {
        terminalView.println("Usage: write <addr> <reg> <val>");
        return;
    }

    const std::string& addrStr = cmd.getSubcommand();
    const std::string& regStr = args[0];
    const std::string& valStr = args[1];

    if (!argTransformer.isValidNumber(addrStr) ||
        !argTransformer.isValidNumber(regStr) ||
        !argTransformer.isValidNumber(valStr)) {
        terminalView.println("Error: Invalid argument. Use decimal or 0x-prefixed hex values.");
        return;
    }

    uint8_t addr = argTransformer.parseHexOrDec(addrStr);
    uint8_t reg  = argTransformer.parseHexOrDec(regStr);
    uint8_t val  = argTransformer.parseHexOrDec(valStr);

    i2cService.beginTransmission(addr);
    i2cService.write(reg);
    i2cService.write(val);
    i2cService.endTransmission();

    terminalView.println("I2C Write: Data Sent.");
}

/*
Read
*/
void I2cController::handleRead(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: read <addr> <reg>");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand()) ||
        !argTransformer.isValidNumber(cmd.getArgs())) {
        terminalView.println("Error: Invalid argument. Use decimal or 0x-prefixed hex values.");
        return;
    }

    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    uint8_t reg  = argTransformer.parseHexOrDec(cmd.getArgs());

    // Write register address first
    i2cService.beginTransmission(addr);
    i2cService.write(reg);
    i2cService.endTransmission(false);

    i2cService.requestFrom(addr, 1);
    if (i2cService.available()) {
        int value = i2cService.read();
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase << value;
        terminalView.println("Read: " + ss.str());
    } else {
        terminalView.println("I2C Read: No data available.");
    }
}

/*
Config
*/
void I2cController::handleConfig() {
    terminalView.println("");
    terminalView.println("I2C Configuration:");

    GlobalState& state = GlobalState::getInstance();

    // SDA
    uint8_t sda = state.getI2cSdaPin();
    while (true) {
        terminalView.print("SDA pin [" + std::to_string(sda) + "]: ");
        std::string sdaInput = getUserInput();
        if (sdaInput.empty()) break;

        if (argTransformer.isValidNumber(sdaInput)) {
            sda = argTransformer.toUint8(sdaInput);
            break;
        } else {
            terminalView.println("Invalid value. Please enter a valid number.");
        }
    }
    state.setI2cSdaPin(sda);

    // SCL
    uint8_t scl = state.getI2cSclPin();
    while (true) {
        terminalView.print("SCL pin [" + std::to_string(scl) + "]: ");
        std::string sclInput = getUserInput();
        if (sclInput.empty()) break;

        if (argTransformer.isValidNumber(sclInput)) {
            scl = argTransformer.toUint8(sclInput);
            break;
        } else {
            terminalView.println("Invalid value. Please enter a valid number.");
        }
    }
    state.setI2cSclPin(scl);

    // Frequency
    uint32_t freq = state.getI2cFrequency();
    while (true) {
        terminalView.print("Frequency [" + std::to_string(freq) + "]: ");
        std::string freqInput = getUserInput();
        if (freqInput.empty()) break;

        if (argTransformer.isValidNumber(freqInput)) {
            freq = argTransformer.toUint32(freqInput);
            break;
        } else {
            terminalView.println("Invalid value. Please enter a valid number.");
        }
    }
    state.setI2cFrequency(freq);

    i2cService.configure(sda, scl, freq);
    terminalView.println("I2C configured.");
    terminalView.println("");
}

/*
Help
*/
void I2cController::handleHelp() {
    terminalView.println("Unknown I2C command. Usage:");
    terminalView.println("  scan");
    terminalView.println("  ping <addr>");
    terminalView.println("  sniff");
    terminalView.println("  read <addr> <reg>");
    terminalView.println("  write <addr> <reg> <val>");
    terminalView.println("  config");
    terminalView.println("  raw instructions, e.g: [0xA0] [0x01 r:8]");
}

std::string I2cController::getUserInput() {
    std::string result;
    while (true) {
        char c = terminalInput.handler();
        if (c == '\r' || c == '\n') break;
        result += c;
        terminalView.print(std::string(1, c));
    }
    terminalView.println("");
    return result;
}

void I2cController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    } else {
        // Reset I2C
        i2cService.end();
        uint8_t sda = state.getI2cSdaPin();
        uint8_t scl = state.getI2cSclPin();
        uint32_t freq = state.getI2cFrequency();
        i2cService.configure(sda, scl, freq);

    }
}