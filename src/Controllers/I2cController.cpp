#include "I2cController.h"

I2cController::I2cController(ITerminalView& terminalView, IInput& terminalInput, 
                             I2cService& i2cService, ArgTransformer& argTransformer, 
                             UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), i2cService(i2cService), argTransformer(argTransformer), userInputManager(userInputManager) {}

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
    terminalView.println("I2C Scan: Scanning I2C bus... Press ENTER to stop");
    terminalView.println("");
    bool found = false;

    for (uint8_t addr = 1; addr < 127; ++addr) {
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("I2C Scan: Cancelled by user.");
            return;
        }
        
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
    terminalView.println("I2C Sniffer: Listening... Press ENTER to stop.\n");
    i2c_sniffer_begin(state.getI2cSclPin(), state.getI2cSdaPin()); // dont need freq to work
    i2c_sniffer_setup();

    std::string line;

    while (true) {
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        while (i2c_sniffer_available()) {
            char c = i2c_sniffer_read();

            if (c == '\n') {
                line += "  ";
                terminalView.print(line);
                line.clear();
            } else {
                line += c;
            }
        }
        delay(5);
    }

    i2c_sniffer_reset_buffer();
    i2c_sniffer_stop();
    terminalView.println("\n\nI2C Sniffer: Stopped.");
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
    terminalView.println("\nI2C Configuration:");

    uint8_t sda = userInputManager.readValidatedUint8("SDA pin", state.getI2cSdaPin());
    state.setI2cSdaPin(sda);

    uint8_t scl = userInputManager.readValidatedUint8("SCL pin", state.getI2cSclPin());
    state.setI2cSclPin(scl);

    uint32_t freq = userInputManager.readValidatedUint32("Frequency", state.getI2cFrequency());
    state.setI2cFrequency(freq);

    i2cService.configure(sda, scl, freq);

    terminalView.println("I2C configured.\n");
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
    terminalView.println("  raw instructions, e.g: [0x13 0x4B r:8]");
}

/*
Config
*/
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