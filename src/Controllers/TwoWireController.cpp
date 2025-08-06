#include "TwoWireController.h"

/*
Constructor
*/
TwoWireController::TwoWireController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    UserInputManager& userInputManager,
    TwoWireService& twoWireService,
    SmartCardShell& smartCardShell
)
    : terminalView(terminalView)
    , terminalInput(terminalInput)
    , userInputManager(userInputManager)
    , twoWireService(twoWireService)
    , smartCardShell(smartCardShell)
{
    // Additional initialization if needed
}

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
    smartCardShell.run();
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
    terminalView.println("  smartcard ");
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


