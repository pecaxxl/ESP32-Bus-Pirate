#include "HdUartController.h"

/*
Constructor
*/
HdUartController::HdUartController(ITerminalView& terminalView, IInput& terminalInput, IInput& deviceInput,
                                   HdUartService& hdUartService, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), deviceInput(deviceInput),
      hdUartService(hdUartService), argTransformer(argTransformer), userInputManager(userInputManager) {}

/*
Entry point for HDUART commands
*/
void HdUartController::handleCommand(const TerminalCommand& cmd) {
    if      (cmd.getRoot() == "bridge") handleBridge();
    else if (cmd.getRoot() == "config") handleConfig();
    else    handleHelp();
}

/*
Entry point for HDUART instructions
*/
void HdUartController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = hdUartService.executeByteCode(bytecodes);
    terminalView.println("");
    terminalView.print("HDUART Read: ");
    terminalView.println(result.empty() ? "No data" : "\n\n" + result);
    terminalView.println("");
}

/*
Bridge mode read/write
*/
void HdUartController::handleBridge() {
    terminalView.println("HDUART Bridge: In progress... Press any ESP32 button to stop.");

    std::string echoBuffer;

    while (true) {
        // Receive UART
        while (hdUartService.available()) {
            char incoming = hdUartService.read();
            if (!echoBuffer.empty() && incoming == echoBuffer.front()) {
                echoBuffer.erase(0, 1); // Filter for echo line
            } else {
                terminalView.print(std::string(1, incoming));
            }
        }

        // Terminal input
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            hdUartService.write(c);
            echoBuffer += c; // echo char
        }

        // Device input
        c = deviceInput.readChar();
        if (c != KEY_NONE) {
            terminalView.println("\nHDUART Bridge: Stopped by user.");
            break;
        }
    }
}

/*
Config
*/
void HdUartController::handleConfig() {
    terminalView.println("\nHDUART Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t pin = userInputManager.readValidatedPinNumber("Shared TX/RX pin", state.getHdUartPin(), forbidden);
    state.setHdUartPin(pin);

    uint32_t baud = userInputManager.readValidatedUint32("Baud rate", state.getHdUartBaudRate());
    state.setHdUartBaudRate(baud);

    uint8_t dataBits = userInputManager.readValidatedUint8("Data bits (5-8)", state.getHdUartDataBits(), 5, 8);
    state.setHdUartDataBits(dataBits);

    char defaultParity = state.getHdUartParity().empty() ? 'N' : state.getHdUartParity()[0];
    char parity = userInputManager.readCharChoice("Parity (N/E/O)", defaultParity, {'N', 'E', 'O'});
    state.setHdUartParity(std::string(1, parity));

    uint8_t stopBits = userInputManager.readValidatedUint8("Stop bits (1 or 2)", state.getHdUartStopBits(), 1, 2);
    state.setHdUartStopBits(stopBits);

    bool inverted = userInputManager.readYesNo("Inverted?", state.isHdUartInverted());
    state.setHdUartInverted(inverted);

    hdUartService.configure(baud, dataBits, parity, stopBits, pin, inverted);

    terminalView.println("HDUART configuration applied.\n");
}

/*
Help
*/
void HdUartController::handleHelp() {
    terminalView.println("\nHDUART Commands:\n"
                         "  bridge       Interactive mode\n"
                         "  config       Set TX/RX pin, baud etc.\n");
}

void HdUartController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
