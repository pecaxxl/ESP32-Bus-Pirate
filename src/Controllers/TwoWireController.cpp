#include "TwoWireController.h"

/*
Constructor
*/
TwoWireController::TwoWireController(ITerminalView& terminalView, IInput& terminalInput)
    : terminalView(terminalView), terminalInput(terminalInput) {}


/*
Entry point for command
*/
void TwoWireController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "sniff") {
        handleSniff();
    } else if (cmd.getRoot() == "config") {
        handleConfig();
    } else {
        terminalView.println("");
        terminalView.println("Unknown 2WIRE command. Usage:");
        terminalView.println("  sniff                - Sniff 8-bit data with start/stop bits [NYI]");
        terminalView.println("  config               - Configure 2WIRE settings [NYI]");
        terminalView.println("  [..]                 - Instruction syntax supported [NYI]");
        terminalView.println("");
    }
}


/*
Entry point for instruction
*/
void TwoWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("Instruction handling not yet implemented");
}


/*
Sniff
*/
void TwoWireController::handleSniff() {
    terminalView.println("2WIRE sniff [NYI]");
}


/*
Config
*/
void TwoWireController::handleConfig() {
    terminalView.println("  [INFO] This mode is not implemented yet.");
    terminalView.println("         Support is planned for a future release.\n");
}


void TwoWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}