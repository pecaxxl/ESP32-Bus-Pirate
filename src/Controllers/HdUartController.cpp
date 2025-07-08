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
    while (true) {
        if (hdUartService.available()) {
            terminalView.print(std::string(1, hdUartService.read()));
        }
        char c = terminalInput.readChar();
        if (c != KEY_NONE) hdUartService.write(c);
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

    uint8_t pin = userInputManager.readValidatedUint8("Shared TX/RX pin", state.getHdUartPin());
    state.setHdUartPin(pin);

    uint32_t baud = userInputManager.readValidatedUint32("Baud rate", state.getHdUartBaudRate());
    state.setHdUartBaudRate(baud);

    uint8_t dataBits = userInputManager.readValidatedUint8("Data bits (5-8)", state.getHdUartDataBits(), 5, 8);
    state.setHdUartDataBits(dataBits);

    char parity = userInputManager.readCharChoice("Parity (N/E/O)", 'N', {'N', 'E', 'O'});
    state.setHdUartParity(std::string(1, parity));

    uint8_t stopBits = userInputManager.readValidatedUint8("Stop bits (1 or 2)", state.getHdUartStopBits(), 1, 2);
    state.setHdUartStopBits(stopBits);

    bool inverted = userInputManager.readYesNo("Inverted?", state.isHdUartInverted());
    state.setHdUartInverted(inverted);

    uint32_t config = buildUartConfig(dataBits, parity, stopBits);
    state.setHdUartConfig(config);

    hdUartService.configure(baud, config, pin, inverted);

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

uint32_t HdUartController::buildUartConfig(uint8_t bits, char parity, uint8_t stop) {
    uint32_t conf = SERIAL_8N1;
    if (bits == 5) conf = (stop == 2) ? SERIAL_5N2 : SERIAL_5N1;
    else if (bits == 6) conf = (stop == 2) ? SERIAL_6N2 : SERIAL_6N1;
    else if (bits == 7) conf = (stop == 2) ? SERIAL_7N2 : SERIAL_7N1;
    else if (bits == 8) conf = (stop == 2) ? SERIAL_8N2 : SERIAL_8N1;

    if (parity == 'E') conf |= 0x02;
    else if (parity == 'O') conf |= 0x01;

    return conf;
}

void HdUartController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
