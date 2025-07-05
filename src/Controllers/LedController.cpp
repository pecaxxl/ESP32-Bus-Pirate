#include "LedController.h"

/*
Constructor
*/
LedController::LedController(ITerminalView& terminalView, IInput& terminalInput, LedService& ledService)
    : terminalView(terminalView), terminalInput(terminalInput), ledService(ledService) {}

/*
Entry point for command
*/
void LedController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "PlaceHolder") {
        // Nothing
    }

    else {
        terminalView.println("");
        terminalView.println("Unknown LED command. Usage:");
        terminalView.println("  ???");
        terminalView.println("");
    }
}

/*
Entry point for instruction
*/
void LedController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("Instruction handling not yet implemented");
}

/*
Config
*/
void LedController::handleConfig() {
    terminalView.println("LED Configuration [NYI]");
}

void LedController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
