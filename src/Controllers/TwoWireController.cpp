#include "TwoWireController.h"

/*
Constructor
*/
TwoWireController::TwoWireController(ITerminalView& terminalView, IInput& terminalInput, UserInputManager& userInputManager, TwoWireService& twoWireService)
    : terminalView(terminalView), terminalInput(terminalInput), userInputManager(userInputManager), twoWireService(twoWireService) {}

/*
Entry point for 2WIRE command
*/
void TwoWireController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "config") {
        handleConfig();
    // Not fully fonctionnal for now
    // } if (cmd.getRoot() == "at") {
    //     handleATR();
    
    // } else if (cmd.getRoot() == "test") {
    //     handleTest();
    // } else if (cmd.getRoot() == "sec") {
    //     handleSniff();
    // } else if (cmd.getRoot() == "dump") {
    //     handleDump();
    } else {
        handleHelp();
    }
}


void TwoWireController::handleTest() {
    ensureConfigured();

    terminalView.println("\n2WIRE TEST: Reading security memory...");
    twoWireService.sendCommand(0x31, 0x00, 0x00);
    auto sec = twoWireService.readResponse(4);
    std::string secStr = "Security:";
    for (auto b : sec) {
        char buf[8];
        sprintf(buf, " 0x%02X", b);
        secStr += buf;
    }
    terminalView.println(secStr);
    delay(2);

    terminalView.println("\n==> Read main memory [0x30 0x00 0x10]");
    twoWireService.sendCommand(0x30, 0x00, 0x10);
    auto main = twoWireService.readResponse(16);
    std::string mainStr = "Main:";
    for (auto b : main) {
        char buf[8];
        sprintf(buf, " 0x%02X", b);
        mainStr += buf;
    }
    terminalView.println(mainStr);
    delay(2);

    terminalView.println("\n==> Read PIN [0x33 0x00 0x03]");
    twoWireService.sendCommand(0x33, 0x00, 0x03);
    auto pin = twoWireService.readResponse(3);
    std::string pinStr = "PIN:";
    for (auto b : pin) {
        char buf[8];
        sprintf(buf, " 0x%02X", b);
        pinStr += buf;
    }
    terminalView.println(pinStr);
}


/*
Entry point for 2WIRE instruction
*/
void TwoWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    // Instruction support à venir
    terminalView.println("[TODO] Instruction support for 2WIRE not yet implemented.");
}

/*
Sniff
*/
void TwoWireController::handleSec() {
    twoWireService.testReadSecurityMemory();
}

/*
Perform ATR
*/
void TwoWireController::handleATR() {
    ensureConfigured();
    terminalView.println("\n2WIRE ATR: Performing...");

    auto atr = twoWireService.performATR();
    std::stringstream ss;
    ss << "ATR: ";

    if (atr.empty()) {
        terminalView.println("No response received.");
    } else {
        terminalView.print("ATR: ");
        for (uint8_t b : atr) {
            ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b) << " ";
        }
        terminalView.println(ss.str());
    }
}

/*
Dump
*/
void TwoWireController::handleDump() {
    ensureConfigured();

    terminalView.println("\n2WIRE DUMP: Reading full main memory (256 bytes)...");

    for (int i = 0; i < 16; ++i) {
        uint8_t addr = i * 16;
        twoWireService.sendCommand(0x30, addr, 16);
        auto data = twoWireService.readResponse(16);

        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        ss << std::setw(2) << addr << ": ";

        for (auto b : data) {
            ss << std::setw(2) << static_cast<int>(b) << " ";
        }

        terminalView.println(ss.str());
    }

    terminalView.println("Done.");
}

/*
Configuration
*/
void TwoWireController::handleConfig() {
    terminalView.println("\nConfigure 2WIRE Pins:");
    const auto& forbidden = state.getProtectedPins();

    uint8_t clk = userInputManager.readValidatedPinNumber("CLK pin", state.getTwoWireClkPin(), forbidden);
    state.setTwoWireClkPin(clk);

    uint8_t io = userInputManager.readValidatedPinNumber("IO pin", state.getTwoWireIoPin(), forbidden);
    state.setTwoWireIoPin(io);

    uint8_t rst = userInputManager.readValidatedPinNumber("RST pin", state.getTwoWireRstPin(), forbidden);
    state.setTwoWireRstPin(rst);

    twoWireService.configure(clk, io, rst);
    configured = true;

    terminalView.println("2WIRE configuration applied.\n");

    terminalView.println("  [INFO] This mode is not implemented yet.");
    terminalView.println("         Support is planned for a future release.\n");
}

/*
Help
*/
void TwoWireController::handleHelp() {
    terminalView.println("\n2WIRE Commands:\n"
                         "  NOT YET FUNCTIONNAL"
                         "  atr           Perform ATR handshake\n"
                         "  dump          Read full 256-byte memory\n"
                         "  config        Set CLK/IO/RST pins\n"
                         "  [..]          Instruction syntax supported later\n");
}

/*
Ensure Configuration
*/
void TwoWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
    } else {
        twoWireService.configure(
            state.getTwoWireClkPin(),
            state.getTwoWireIoPin(),
            state.getTwoWireRstPin()
        );
    }
}


