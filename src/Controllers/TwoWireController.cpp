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
    if      (cmd.getRoot() == "config")    handleConfig();
    else if (cmd.getRoot() == "sniff")     handleSniff();
    else if (cmd.getRoot() == "smartcard") handleSmartCard(cmd);
    else                                   handleHelp();

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

    if      (sub == "probe")    handleSmartCardProbe();
    else if (sub == "security") handleSmartCardSecurity();
    else if (sub == "dump")     handleSmartCardDump();
    else if (sub == "unlock")   handleSmartCardUnlock();
    else if (sub == "psc")      handleSmartCardPsc(cmd);
    else if (sub == "write")    handleSmartCardWrite(cmd);
    else if (sub == "protect")  handleSmartCardProtect();
    else {
        terminalView.println("Unknown smartcard subcommand. Usage:");
        terminalView.println("  smartcard probe");
        terminalView.println("  smartcard security");
        terminalView.println("  smartcard dump");
        terminalView.println("  smartcard unlock");
        terminalView.println("  smartcard protect");
        terminalView.println("  smartcard psc [get]");
        terminalView.println("  smartcard psc set");
        terminalView.println("  smartcard write");
    }
}

/*
Smartcard Security
*/
void TwoWireController::handleSmartCardSecurity() {
    ensureConfigured();
    twoWireService.resetSmartCard();

    terminalView.println("2WIRE Security: Perfoming...\n");
    
    // Security memory
    terminalView.println("   [Security Memory] Command: 0x31 0x00 0x00");
    twoWireService.sendCommand(0x31, 0x00, 0x00);
    auto sec = twoWireService.readResponse(4);

    // Validate
    bool allZero = std::all_of(sec.begin(), sec.end(), [](uint8_t b) { return b == 0x00; });
    bool allFF   = std::all_of(sec.begin(), sec.end(), [](uint8_t b) { return b == 0xFF; });
    if (sec.empty() || allZero || allFF) {
        terminalView.println("2WIRE Security: ❌ No smartcard detected (response invalid)");
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

    terminalView.println("\n2WIRE Security: ✅ Completed.");
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
        terminalView.println("2WIRE ATR: ❌ No response received from smartcard");
        return;
    }

    // Decode and display
    auto decodedAtr = twoWireService.parseSmartCardAtr(atr);
    for (uint8_t b : atr) {
        ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b) << " ";
    }
    terminalView.println(decodedAtr);
    
    twoWireService.resetSmartCard();
    terminalView.println("2WIRE ATR: ✅ Completed.");
}

/*
Smartcard Dump
*/
void TwoWireController::handleSmartCardDump() {
    twoWireService.resetSmartCard();
    delay(10);
    terminalView.println("\n2WIRE Dump: Reading full memory (MAIN + SEC + PROTECT)...");

    // Dump 256 bytes + sec mem + protection mem
    auto dump = twoWireService.dumpSmartCardFullMemory();
    if (dump.size() != 264) {
        terminalView.println("\n2WIRE Dump: ❌ Failed, unexpected size.");
        return;
    }

    // Validate data
    bool allZero = std::all_of(dump.begin(), dump.end(), [](uint8_t b) { return b == 0x00; });
    bool allFF   = std::all_of(dump.begin(), dump.end(), [](uint8_t b) { return b == 0xFF; });
    if (allZero || allFF) {
        terminalView.println("\n2WIRE Dump: ❌ Smartcard is blank or no smartcard detected");
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

    twoWireService.resetSmartCard();
    terminalView.println("\n2WIRE Dump: ✅ Completed.");
}

/*
Smartcard Protect
*/
void TwoWireController::handleSmartCardProtect() {
    twoWireService.resetSmartCard();
    terminalView.println("⚠️ The smartcard will PERMANENTLY disable writes to main memory.");
    bool confirm = userInputManager.readYesNo("Are you sure you want to lock it PERMANENTLY?", false);
    if (!confirm) {
        terminalView.println("\n❌ Lock cancelled.");
        return;
    }

    bool ok = twoWireService.protectSmartCard();
    if (ok) terminalView.println("\n✅ Smartcard successfully locked (writes disabled).");
    else    terminalView.println("\n❌ Failed to lock smartcard.");
}

/*
Smartcard Unlock
*/
void TwoWireController::handleSmartCardUnlock() {
    twoWireService.resetSmartCard();
    terminalView.println("2WIRE Unlock: Attempting unlock procedure...");

    std::string psc = userInputManager.readValidatedHexString("Enter PSC (PIN Code) (ex: 123456)", 3);
    if (psc.length() != 6) {
        terminalView.println("PSC (PIN Code) must be 3 bytes (6 hex chars)");
        return;
    }

    uint8_t pscBytes[3] = {
        static_cast<uint8_t>(std::stoi(psc.substr(0, 2), nullptr, 16)),
        static_cast<uint8_t>(std::stoi(psc.substr(2, 2), nullptr, 16)),
        static_cast<uint8_t>(std::stoi(psc.substr(4, 2), nullptr, 16))
    };

    bool success = twoWireService.unlockSmartCard(pscBytes);

    if (success) {
        terminalView.println("\n✅ Unlock successful: Access to main memory granted.");
    } else {
        terminalView.println("\n❌ Unlock failed: PSC incorrect or no attempts remaining.");
    }

    // Number of attempts left
    auto secAfter = twoWireService.readSmartCardSecurityMemory();
    if (!secAfter.empty()) {
        uint8_t attempts = twoWireService.parseSmartCardRemainingAttempts(secAfter[0]);
        terminalView.println("→ Remaining Attempts: " + std::to_string(attempts));
    }
}

/*
Smartcard PSC (PIN Code)
*/
void TwoWireController::handleSmartCardPsc(const TerminalCommand& cmd) {
    twoWireService.resetSmartCard();
    std::string arg = cmd.getArgs();

    if (arg.empty()) {
        arg = "get"; // Default to "get" if no argument provided
    }

    // GET PSC
    if (arg == "get") {
        uint8_t psc[3];
        bool ok = twoWireService.getSmartCardPSC(psc);
        if (ok) {
            terminalView.println("ℹ️  Note: The PSC (PIN Code) can only be read if the smartcard is unlocked.");
            std::stringstream ss;
            ss << "🔐 Current PSC (PIN Code): ";
            for (int i = 0; i < 3; ++i)
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)psc[i] << " ";
            terminalView.println(ss.str());

        } else {
            terminalView.println("\n❌ Failed to read PSC (PIN Code).");
        }

    // SET PSC
    } else if (arg == "set") {
        std::string newPscStr = userInputManager.readValidatedHexString("Enter new 6 hex digit PSC (PIN Code)", 3);
        if (newPscStr.length() != 6) {
            terminalView.println("PSC (PIN Code) must be 6 hex digits");
            return;
        }

        uint8_t newPsc[3] = {
            static_cast<uint8_t>(std::stoi(newPscStr.substr(0, 2), nullptr, 16)),
            static_cast<uint8_t>(std::stoi(newPscStr.substr(2, 2), nullptr, 16)),
            static_cast<uint8_t>(std::stoi(newPscStr.substr(4, 2), nullptr, 16))
        };

        bool ok = twoWireService.updateSmartCardPSC(newPsc);
        if (ok) {
            terminalView.println("\n✅ PSC (PIN Code) updated successfully.");
        } else {
            terminalView.println("\nℹ️  Note: The PSC (PIN Code) can only be set if the smartcard is unlocked.");
            terminalView.println("❌ Failed to update PSC (PIN Code).");
        }
    }
}

// Smartcard Write
void TwoWireController::handleSmartCardWrite(const TerminalCommand&) {
    twoWireService.resetSmartCard();

    int offset = userInputManager.readValidatedUint8("Enter offset (0–255 or 0x..)", 0);
    if (offset < 0 || offset >= 256) {
        terminalView.println("\n❌ Invalid offset (must be between 0 and 255).");
        return;
    }

    int data = userInputManager.readValidatedUint8("Enter data byte (0–255 or 0x..)", 0);
    if (data < 0 || data > 0xFF) {
        terminalView.println("\n❌ Invalid data byte.");
        return;
    }

    bool ok = twoWireService.writeSmartCardMainMemory(static_cast<uint8_t>(offset), static_cast<uint8_t>(data));
    if (ok) terminalView.println("\n✅ Write successful.");
    else    terminalView.println("\n❌ Write failed.");
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

    terminalView.println("2WIRE configuration applied.\n");
}

/*
Help
*/
void TwoWireController::handleHelp() {
    terminalView.println("Unknown 2Wire command. Usage:");
    terminalView.println("  config");
    terminalView.println("  sniff [NYI]");
    terminalView.println("  smartcard probe");
    terminalView.println("  smartcard security");
    terminalView.println("  smartcard dump");
    terminalView.println("  smartcard write");
    terminalView.println("  smartcard unlock");
    terminalView.println("  smartcard protect");
    terminalView.println("  smartcard psc [get]");
    terminalView.println("  smartcard psc set");
    terminalView.println("  [0xAB r:4] Instruction syntax [NYI]");
}

/*
Ensure Configuration
*/
void TwoWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    } 

    twoWireService.configure(
        state.getTwoWireClkPin(),
        state.getTwoWireIoPin(),
        state.getTwoWireRstPin()
    );
}


