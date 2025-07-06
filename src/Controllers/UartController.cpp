#include "UartController.h"

/*
Constructor
*/
UartController::UartController(ITerminalView& terminalView, IInput& terminalInput, IInput& deviceInput, 
                               UartService& uartService, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), deviceInput(deviceInput), uartService(uartService), argTransformer(argTransformer), userInputManager(userInputManager) {}


/*
Entry point for command
*/
void UartController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "scan") {
        handleScan();
    }

    else if (cmd.getRoot() == "ping") {
        handlePing();
    }

    else if (cmd.getRoot() == "read") {
        handleRead();
    } 

    else if (cmd.getRoot() == "write") {
        handleWrite(cmd);
    } 
    
    else if (cmd.getRoot() == "bridge") {
        handleBridge();
    } 

    else if (cmd.getRoot() == "glitch") {
        handleGlitch();
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
void UartController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = uartService.executeByteCode(bytecodes);
    terminalView.println("");
    terminalView.print("UART Read: ");
    if (!result.empty()) {
        terminalView.println("");
        terminalView.println("");
        terminalView.println(result);
        uartService.clearUartBuffer();
    } else {
        terminalView.print("No data");
    }
    terminalView.println("");
}

/*
Bridge
*/
void UartController::handleBridge() {
    terminalView.println("Uart Bridge: in progress... Press Cardputer key to stop");
    while (true) {
        // Read from UART and print to terminal
        if (uartService.available()) {
            char c = uartService.read();
            if (c == '\n') {
                terminalView.println("");  // saut de ligne
            } else {
                terminalView.print(std::string(1, c));
            }
        }

        // Read from user input and write to UART
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            uartService.write(c);
        }
        
        // Read from device input and stop bridge if any
        c = deviceInput.readChar();
        if (c != KEY_NONE) {  
            terminalView.println("Uart Bridge: Stopped by user.");
            break;
        }
    }
}

/*
Read
*/
void UartController::handleRead() {
    terminalView.println("UART Read: Streaming until ENTER is pressed...");
    uartService.flush();

    while (true) {
        // Stop if ENTER is pressed
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("");
            terminalView.println("UART Read: Stopped by user.");
            break;
        }

        // Print UART data as it comes
        while (uartService.available() > 0) {
            char c = uartService.read();
            terminalView.print(std::string(1, c));
        }
    }
}

/*
Write
*/
void UartController::handleWrite(TerminalCommand cmd) {
    uartService.print(cmd.getSubcommand() + cmd.getArgs());
    terminalView.println("UART Write: Text sent at baud " + std::to_string(state.getUartBaudRate()));
}

/*
Ping
*/
void UartController::handlePing() { 
    std::string response;
    unsigned long start = millis();
    size_t probeIndex = 0;

    terminalView.println("UART Ping: probing for 5 seconds...");
    uartService.clearUartBuffer();

    while (millis() - start < 5000) {
        // Envoi progressif
        if (probeIndex < probes.size()) {
            uartService.write(probes[probeIndex]);
            probeIndex++;
        }

        // Lecture continue
        char c;
        while (uartService.available() > 0) {
            c = uartService.read();
            response += c;
        }

        delay(10);
    }

    // Analyse ASCII simple
    size_t asciiCount = 0;
    std::string result = "";
    for (char c : response)
        if (isprint(static_cast<unsigned char>(c)) || isspace(c)) {
            asciiCount++;
            result += c;
        }

    if (asciiCount < 5) {
        terminalView.println("UART Ping: No response.");
        return;
    }

    terminalView.println("UART Response: ");
    terminalView.println("");
    terminalView.println(result);
    terminalView.println("");

    terminalView.println("UART Ping: Device detected");
}

/*
Scan
*/
void UartController::handleScan() {
    std::vector<int> baudrates = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};

    terminalView.println("UART Scan: in progress... (press ENTER to cancel)");
    uartService.clearUartBuffer();

    for (int baud : baudrates) {
        uartService.switchBaudrate(baud);
        terminalView.println("→ Testing baudrate " + std::to_string(baud));

        std::string response;
        size_t asciiCount = 0;
        size_t probeIndex = 0;
        unsigned long start = millis();
        unsigned long lastSend = 0;

        uartService.flush();
        while (millis() - start < 1000) {
            // Cancel if user presses Enter
            char key = terminalInput.readChar();
            if (key == '\r' || key == '\n') {
                terminalView.println("UART Scan: Cancelled by user.");
                return;
            }

            // Send probes to get uart data
            if (probeIndex < probes.size()) {
                uartService.write(probes[probeIndex]);
                probeIndex++;
                lastSend = millis();
            }

            // Read incoming UART data
            while (uartService.available() > 0) {
                char c = uartService.read();
                response += c;

                if (isprint(c) || isspace(c))
                    asciiCount++;
            }

            float asciiRatio = response.empty() ? 0.0f : (float)asciiCount / response.size();

            // Accept short but readable responses (e.g., "AT+OK\r\n")
            bool validShort = response.length() >= 10 && asciiRatio >= 0.95f;

            // Accept long and mostly printable responses
            bool validLong = response.length() > 60 && asciiCount > 20 && asciiRatio >= 0.9f;
            
            // Baudrate detected
            if (validShort || validLong) {
                terminalView.println("");
                terminalView.println("Preview:");
                terminalView.println(response.substr(0, 100) + "...");
                uartService.switchBaudrate(baud);
                state.setUartBaudRate(baud);
                terminalView.println("");
                terminalView.println("UART Scan: Setting baudrate to UART config.");
                terminalView.println("UART Scan: Baudrate detected " + std::to_string(baud));
                return;
            }

            delay(10);
        }
    }

    terminalView.println("Uart Scan: No device detected.");
}

/*
Config
*/
void UartController::handleConfig() {
    terminalView.println("");
    terminalView.println("UART Configuration:");

    GlobalState& state = GlobalState::getInstance();

    uint8_t rxPin = userInputManager.readValidatedUint8("RX pin number", state.getUartRxPin());
    state.setUartRxPin(rxPin);

    uint8_t txPin = userInputManager.readValidatedUint8("TX pin number", state.getUartTxPin());
    state.setUartTxPin(txPin);

    uint32_t baud = userInputManager.readValidatedUint32("Baud rate", state.getUartBaudRate());
    state.setUartBaudRate(baud);

    uint8_t dataBits = userInputManager.readValidatedUint8("Data bits (5-8)", state.getUartDataBits(), 5, 8);
    state.setUartDataBits(dataBits);

    char defaultParity = state.getUartParity().empty() ? 'N' : state.getUartParity()[0];
    char parityChar = userInputManager.readCharChoice("Parity (N/E/O)", defaultParity, {'N', 'E', 'O'});
    state.setUartParity(std::string(1, parityChar));

    uint8_t stopBits = userInputManager.readValidatedUint8("Stop bits (1 or 2)", state.getUartStopBits(), 1, 2);
    state.setUartStopBits(stopBits);

    bool inverted = userInputManager.readYesNo("Inverted?", state.isUartInverted());
    state.setUartInverted(inverted);

    uint32_t config = buildUartConfig(dataBits, parityChar, stopBits);
    state.setUartConfig(config);

    uartService.configure(baud, config, rxPin, txPin, inverted);

    terminalView.println("UART configuration applied.");
    terminalView.println("");
}

/*
Help
*/
void UartController::handleHelp() {
    terminalView.println("");
    terminalView.println("Unknown UART command. Usage:");
    terminalView.println("  scan");
    terminalView.println("  ping");
    terminalView.println("  read");
    terminalView.println("  write <text>");
    terminalView.println("  bridge");
    terminalView.println("  glitch");
    terminalView.println("  config");
    terminalView.println("  raw instructions, ['AT' D:100 r:128]");
    terminalView.println("");
}

/*
Glitch
*/
void UartController::handleGlitch() {
    terminalView.println("Uart Glicher: Not Yet Implemented");
}


/*
Ensure Config
*/
void UartController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}

/*
Utils
*/
uint32_t UartController::buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits) {
    uint32_t config = SERIAL_8N1;
    if (dataBits == 5) config = (stopBits == 2) ? SERIAL_5N2 : SERIAL_5N1;
    else if (dataBits == 6) config = (stopBits == 2) ? SERIAL_6N2 : SERIAL_6N1;
    else if (dataBits == 7) config = (stopBits == 2) ? SERIAL_7N2 : SERIAL_7N1;
    else if (dataBits == 8) config = (stopBits == 2) ? SERIAL_8N2 : SERIAL_8N1;

    if (parity == 'E') config |= 0x02;
    else if (parity == 'O') config |= 0x01;

    return config;
}
