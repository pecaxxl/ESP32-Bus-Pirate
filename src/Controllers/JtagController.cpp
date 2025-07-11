#include "JtagController.h"

/*
Constructor
*/
JtagController::JtagController(ITerminalView& terminalView, IInput& terminalInput)
    : terminalView(terminalView), terminalInput(terminalInput) {}

void JtagController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "scan") {
        handleScan();
    } 
    
    else {
        terminalView.println("");
        terminalView.println("Unknown JTAG command. Usage:");
        terminalView.println("  scan");
        terminalView.println("");
    }
}

/*
Scan
*/
void JtagController::handleScan() {
    terminalView.println("BlueTag JTAG pinout scan [NYI]");
}

/*
Config
*/
void JtagController::handleConfig() {
    terminalView.println("  [INFO] This mode is not implemented yet.");
    terminalView.println("         Support is planned for a future release.\n");
}


void JtagController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}