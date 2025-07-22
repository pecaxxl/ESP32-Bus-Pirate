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
    } else if (cmd.getRoot() == "sniff") {
        handleSniff();
    } else if (cmd.getRoot() == "smartcard") {
        handleSmartCard(cmd);
    } else {
        handleHelp();
    }
}

/*
Entry point for 2WIRE instruction
*/
void TwoWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("[TODO] Instruction support for 2WIRE not yet implemented.");
}

/*
Sniff
*/
void TwoWireController::handleSniff() {
    terminalView.println("2WIRE Sniff [NYI]\n");
}

/*
Smartcard
*/
void TwoWireController::handleSmartCard(const TerminalCommand& cmd) {
    std::string sub = cmd.getSubcommand();

    if (sub == "probe") {
        handleSmartCardProbe();
    } else if (sub == "security") {
        handleSmartCardSecurity();
    } else if (sub == "dump") {
        handleSmartCardDump();
    } else {
        terminalView.println("Unknown smartcard subcommand. Usage:");
        terminalView.println("  smartcard probe");
        terminalView.println("  smartcard security");
        terminalView.println("  smartcard dump");
    }
}

/*
Smartcard Security
*/
void TwoWireController::handleSmartCardSecurity() {
    ensureConfigured();

    terminalView.println("2WIRE Security: Perfoming...\n");
    
    // Security memory
    terminalView.println("   [Security Memory] Command: 0x31 0x00 0x00");
    twoWireService.sendCommand(0x31, 0x00, 0x00);
    auto sec = twoWireService.readResponse(4);

    // Validate
    bool allZero = std::all_of(sec.begin(), sec.end(), [](uint8_t b) { return b == 0x00; });
    bool allFF   = std::all_of(sec.begin(), sec.end(), [](uint8_t b) { return b == 0xFF; });
    if (sec.empty() || allZero || allFF) {
        terminalView.println("2WIRE Security: No smartcard detected (response invalid)");
        return;
    }
    
    // Display
    std::stringstream secOut;
    secOut << "   Security Bytes: ";
    for (auto b : sec) secOut << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
    terminalView.println(secOut.str());

    if (!sec.empty()) {
        uint8_t attempts = twoWireService.parseSmartCardRemainingAttempts(sec[0]);
        terminalView.println("   Remaining Unlock Attempts: " + std::to_string(attempts));
    }

    terminalView.println("\n2WIRE Security: Completed.");
}

/*
Smartcard Probe (ATR)
*/
void TwoWireController::handleSmartCardProbe() {
    terminalView.println("\n2WIRE ATR: Performing...\n");

    // ATR
    auto atr = twoWireService.performSmartCardAtr();
    std::stringstream ss;
    ss << "ATR: ";

    // Validate
    if (atr.empty() || atr[0] == 0x00 || atr[0] == 0xFF) {
        terminalView.println("2WIRE ATR: No response received from smartcard");
        return;
    }

    // Decode and display
    auto decodedAtr = twoWireService.parseSmartCardAtr(atr);
    for (uint8_t b : atr) {
        ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b) << " ";
    }
    terminalView.println(decodedAtr);
    terminalView.println("2WIRE ATR: Completed.");
}

/*
Smartcard Dump
*/
void TwoWireController::handleSmartCardDump() {
    terminalView.println("\n2WIRE Dump: Reading full memory (MAIN + SEC + PROTECT)...");

    // Dump 256 bytes + sec mem + protection mem
    auto dump = twoWireService.dumpSmartCardFullMemory();
    if (dump.size() != 264) {
        terminalView.println("2WIRE Dump: Failed, unexpected size.");
        return;
    }

    // Validate data
    bool allZero = std::all_of(dump.begin(), dump.end(), [](uint8_t b) { return b == 0x00; });
    bool allFF   = std::all_of(dump.begin(), dump.end(), [](uint8_t b) { return b == 0xFF; });
    if (allZero || allFF) {
        terminalView.println("2WIRE Dump: Smartcard is blank or no smartcard detected");
        return;
    }

    // Main memory (0–255)
    terminalView.println("\n[Main Memory]");
    for (int i = 0; i < 256; i += 16) {
        std::stringstream line;
        line << std::hex << std::uppercase << std::setfill('0');
        line << std::setw(2) << i << ": ";
        for (int j = 0; j < 16; ++j) {
            line << std::setw(2) << static_cast<int>(dump[i + j]) << " ";
        }
        terminalView.println(line.str());
    }

    // Security memory (256–259)
    terminalView.println("\n[Security Memory]");
    std::stringstream sec;
    sec << "SEC: ";
    for (int i = 256; i < 260; ++i) {
        sec << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(dump[i]) << " ";
    }
    uint8_t attempts = twoWireService.parseSmartCardRemainingAttempts(dump[256]);
    sec << "→ Attempts Left: " << std::dec << (int)attempts;
    terminalView.println(sec.str());

    // Protection memory (260–263)
    terminalView.println("\n[Protection Memory]");
    std::stringstream prt;
    prt << "PRT: ";
    for (int i = 260; i < 264; ++i) {
        prt << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(dump[i]) << " ";
    }
    terminalView.println(prt.str());

    terminalView.println("\n2WIRE Dump: Completed.");
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
    terminalView.println("Unknown 2Wire command. Usage:");
    terminalView.println("  config");
    terminalView.println("  sniff");
    terminalView.println("  smartcard probe");
    terminalView.println("  smartcard security");
    terminalView.println("  smartcard dump");
    terminalView.println("  [0xAB r:4] Instruction syntax [NYI]");
    
}

/*
Ensure Configuration
*/
void TwoWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    } 

    twoWireService.configure(
        state.getTwoWireClkPin(),
        state.getTwoWireIoPin(),
        state.getTwoWireRstPin()
    );
}


