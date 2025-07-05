#include "SpiController.h"

SpiController::SpiController(ITerminalView& terminalIiew, IInput& terminalInput, SpiService& service, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), spiService(service), argTransformer(argTransformer) {}

/*
Entry point for command
*/
void SpiController::handleCommand(const TerminalCommand& cmd) {

    if (cmd.getRoot() == "sniff") {
        handleSniff();
    }

    else if (cmd.getRoot() == "flash") {
        if (cmd.getSubcommand() == "probe") {
            handleFlashProbe();
        } else if (cmd.getSubcommand() == "read") {
            handleFlashRead();
        } else if (cmd.getSubcommand() == "write") {
            handleFlashWrite();
        } else if (cmd.getSubcommand() == "erase") {
            handleFlashErase();
        } else {
            terminalView.println("Unknown SPI flash command. Use: probe, read, write, erase");
        }
    } 

    else if (cmd.getRoot() == "config") {
        handleConfig();
    }
    
    else {
        terminalView.println("");
        terminalView.println("Unknown SPI command. Usage:");
        terminalView.println("  sniff");
        terminalView.println("  flash probe");
        terminalView.println("  flash read");
        terminalView.println("  flash write");
        terminalView.println("  flash erase");
        terminalView.println("  config");
        terminalView.println("  [..]                 - Instruction syntax supported [NYI]");
        terminalView.println("");
    }
}

/*
Entry point for instructions
*/
void SpiController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("SPI Instruction [NYI]");
}

/*
Sniff
*/
void SpiController::handleSniff() {
        terminalView.println("SPI sniff [NYI]");
}

/*
Flash Probe
*/
void SpiController::handleFlashProbe() {
    terminalView.println("SPI flash probe [NYI]");
}

/*
Flash Read
*/
void SpiController::handleFlashRead() {
    terminalView.println("SPI flash read [NYI]");
}

/*
Flash Write
*/
void SpiController::handleFlashWrite() {
    terminalView.println("SPI flash write [NYI]");
}

/*
Flash Erase
*/
void SpiController::handleFlashErase() {
    terminalView.println("SPI flash erase [NYI]");
}

/*
Config
*/
void SpiController::handleConfig() {
    terminalView.println("SPI config [NYI]");
}

std::string SpiController::getUserInput() {
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

void SpiController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
