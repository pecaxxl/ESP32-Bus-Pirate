#include "UartController.h"

/*
Constructor
*/
UartController::UartController(ITerminalView& terminalView, IInput& terminalInput, IInput& deviceInput, 
                               UartService& uartService, HdUartService& hDUartService, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), deviceInput(deviceInput), uartService(uartService), hdUartService(hdUartService), argTransformer(argTransformer), userInputManager(userInputManager) {}


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
    terminalView.println("Uart Bridge: In progress... Press any ESP32 button to stop.\n");
    while (true) {
        // Read from UART and print to terminal
        if (uartService.available()) {
            char c = uartService.read();
            terminalView.print(std::string(1, c));
        }

        // Read from user input and write to UART
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            uartService.write(c);
        }
        
        // Read from device input and stop bridge if any
        c = deviceInput.readChar();
        if (c != KEY_NONE) {  
            terminalView.println("\nUart Bridge: Stopped by user.");
            break;
        }
    }
}

/*
Read
*/
void UartController::handleRead() {
    terminalView.println("UART Read: Streaming until [ENTER] is pressed...");
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
    std::string raw = cmd.getSubcommand() + cmd.getArgs();
    std::string decoded = argTransformer.decodeEscapes(raw);
    uartService.print(decoded);
    terminalView.println("UART Write: Text sent at baud " + std::to_string(state.getUartBaudRate()));
}

/*
Ping
*/
void UartController::handlePing() { 
    std::string response;
    unsigned long start = millis();
    size_t probeIndex = 0;

    terminalView.println("UART Ping: Probing for 5 seconds...");
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
    terminalView.println("UART Scan: in progress... Press [ENTER] to cancel");
    terminalView.println("");
    terminalView.println("[INFOS]");
    terminalView.println("  The UART scanner attempts to detect the correct baudrate");
    terminalView.println("  by iteratively switching speeds, sending predefined probes");
    terminalView.println("");

    uartService.clearUartBuffer();
    scanCancelled = false;

    for (int baud : baudrates) {
        if (scanCancelled) return;
        if (scanAtBaudrate(baud)) {
            state.setUartBaudRate(baud);
            uartService.switchBaudrate(baud);
            terminalView.println("");
            terminalView.println("UART Scan: Setting baudrate to UART config.");
            terminalView.println("UART Scan: Baudrate detected " + std::to_string(baud));
            terminalView.println("");
            return;
        }
    }

    uartService.switchBaudrate(state.getUartBaudRate()); // restore previous
    terminalView.println("Uart Scan: No device detected.");
    terminalView.println("");
}

bool UartController::scanAtBaudrate(int baud) {
    const size_t maxResponseSize = 8192;
    uartService.switchBaudrate(baud);
    terminalView.println("→ Testing baudrate " + std::to_string(baud));
    uartService.clearUartBuffer();

    std::string response;
    size_t asciiCount = 0;
    size_t probeIndex = 0;
    unsigned long start = millis();

    while (millis() - start < 1500) {
        if (checkScanCancelled()) return false;
        sendNextProbe(probeIndex);
        updateResponse(response, asciiCount, maxResponseSize);

        if (isValidResponse(response, asciiCount)) {
            terminalView.println("");
            terminalView.println("Preview:");
            auto cleaned = argTransformer.filterPrintable(response.substr(0, 100));
            terminalView.println(cleaned + "...");
            return true;
        }

        delay(10);
    }

    return false;
}

bool UartController::checkScanCancelled() {
    char key = terminalInput.readChar();
    if (key == '\r' || key == '\n') {
        terminalView.println("UART Scan: Cancelled by user.");
        scanCancelled = true;
        return true;
    }
    return false;
}

void UartController::sendNextProbe(size_t& probeIndex) {
    if (probeIndex < probes.size()) {
        uartService.write(probes[probeIndex]);
        probeIndex++;
    }
}

void UartController::updateResponse(std::string& response, size_t& asciiCount, size_t maxSize) {
    unsigned long readStart = millis();
    const unsigned long readTimeout = 150;  // ms
    while (uartService.available() > 0 && millis() - readStart < readTimeout) {
        char c = uartService.read();

        if (response.length() >= maxSize) {
            char dropped = response.front();
            if (isprint(dropped) || isspace(dropped)) asciiCount--;
            response.erase(0, 1);
        }

        if (isprint(c) || isspace(c)) asciiCount++;
        response += c;
    }
}

bool UartController::isValidResponse(const std::string& response, size_t asciiCount) {
    if (response.empty()) return false;

    float ratio = static_cast<float>(asciiCount) / response.size();
    float entropy = computeEntropy(response);

    bool plausibleLength = response.length() >= 32;
    bool readableEnough = ratio >= 0.85f;
    bool entropyOK = entropy >= 3.0f && entropy <= 7.5f;

    return plausibleLength && readableEnough && entropyOK;
}

float UartController::computeEntropy(const std::string& data) {
    std::unordered_map<char, size_t> freq;
    for (char c : data)
        freq[c]++;

    float entropy = 0.0f;
    for (auto& p : freq) {
        float prob = static_cast<float>(p.second) / data.size();
        entropy -= prob * std::log2(prob);
    }
    return entropy;
}

/*
Config
*/
void UartController::handleConfig() {
    terminalView.println("");
    terminalView.println("UART Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t rxPin = userInputManager.readValidatedPinNumber("RX pin number", state.getUartRxPin(), forbidden);
    state.setUartRxPin(rxPin);

    uint8_t txPin = userInputManager.readValidatedPinNumber("TX pin number", state.getUartTxPin(), forbidden);
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

    uint32_t config = uartService.buildUartConfig(dataBits, parityChar, stopBits);
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
    // hdUartService.end(); // it crashes the app for some reasons
    if (!uartService.getConfigured()) {
        handleConfig();
    }
}