#include "ThreeWireController.h"

/*
Constructor
*/
ThreeWireController::ThreeWireController(ITerminalView& terminalView, IInput& terminalInput)
    : terminalView(terminalView), terminalInput(terminalInput) {}

/*
Entry point for command
*/
void ThreeWireController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "config") {
        handleConfig();
    } else {
        terminalView.println("");
        terminalView.println("Unknown 3WIRE command. Usage:");
        terminalView.println("  [..]                 - [NYI]");
        terminalView.println("");
    }
}

/*
Entry point for instructions
*/
void ThreeWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("Instruction handling not yet implemented");
}

/*
Config
*/
void ThreeWireController::handleConfig() {
    terminalView.println("  [INFO] This mode is not implemented yet.");
    terminalView.println("         Support is planned for a future release.\n");
}

void ThreeWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}