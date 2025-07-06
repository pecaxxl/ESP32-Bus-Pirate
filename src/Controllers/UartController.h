#pragma once

#include <vector>
#include <string>
#include "HardwareSerial.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Services/UartService.h"
#include "Views/ITerminalView.h"
#include "Inputs/IInput.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"

class UartController {
public:
    // Constructor
    UartController(ITerminalView& terminalView, 
                   IInput& terminalInput,
                   IInput& deviceInput,
                   UartService& uartService, 
                   ArgTransformer& argTransformer,
                   UserInputManager& userInputManager);
    
    // Entry pooint for UART command
    void handleCommand(const TerminalCommand& cmd);

    //  Entry point for handle parsed bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);
    
    // Ensure UART is configured before use
    void ensureConfigured();
    
    private:
    // Start bidirectional UART bridge
    void handleBridge();
    
    // Perform a simple read
    void handleRead();
    
    // Probe known devices
    void handleScan();
    
    // Send simple ping command
    void handlePing();
    
    // Write data to UART
    void handleWrite(TerminalCommand cmd);

    // Configure UART settings
    void handleConfig();

    // Display available commands
    void handleHelp();

    // Send glitch pattern over uart
    void handleGlitch();

    // Encode UART config bits
    uint32_t buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits);

    ITerminalView& terminalView;
    IInput& terminalInput;
    IInput& deviceInput;
    UartService& uartService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;

    // Predefined probe commands for scan()
    std::vector<std::string> probes = {
        // Generic AT commands
        "AT\r\n",
        "ATE0\r\n", "ATE1\r\n", "ATI\r\n", "ATI0\r\n", "ATI1\r\n", "ATI2\r\n",
        "AT+GMR\r\n", "AT+VERSION?\r\n", "AT+HELP\r\n",

        // Modem/GSM specific
        "AT+CSQ\r\n", "AT+CREG?\r\n", "AT+CGMI\r\n", "AT+CGMM\r\n",
        "AT+CGMR\r\n", "AT+CGSN\r\n",

        // GPS modules
        "$PMTK?\r\n", "$GPTXT\r\n",

        // Miscellaneous
        "help\r\n", "?\r\n", "\r\n",
        "\x1B", "\x03", "\x04", "\x1A", "\x11", "\x13"  // ESC, Ctrl+C/Z/D, XON/XOFF
    };
};
