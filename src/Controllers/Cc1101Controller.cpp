#include "Controllers/CC1101Controller.h"
//#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include <sstream>
#include <algorithm>

/*
Constructor
*/
CC1101Controller::CC1101Controller(ITerminalView& terminalView, IInput& terminalInput, CC1101Service& cc1101Service, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), cc1101Service(cc1101Service), argTransformer(argTransformer), userInputManager(userInputManager){}

/*
Entry point to handle a CC1101 commands
*/
void CC1101Controller::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "sniff") ;//handleSniff(cmd);
  
    else if (cmd.getRoot() == "config") {
        handleConfig();
    } 
        else if (cmd.getRoot() == "send") {
        handleSend(cmd);
    } 
    
    else {
        handleHelp();
    }
    
}

void CC1101Controller::handleSend(TerminalCommand cmd) {
    std::string raw = cmd.getSubcommand() + cmd.getArgs();
    std::string decoded = argTransformer.decodeEscapes(raw);
        if (decoded.empty()) {
        terminalView.println("No data to send. Usage: send <text>");
        return;
    }
    //uartService.print(decoded);
    cc1101Service.send(decoded);
    terminalView.println("CC1101 Sent: " + decoded);
}

/*
Config
*/
void CC1101Controller::handleConfig() {
    terminalView.println("");
    terminalView.println("CC1101 Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t cs = userInputManager.readValidatedPinNumber("CC1101 CS pin", state.getCC1101CSPin(), forbidden);
    state.setCC1101CSPin(cs);

    uint8_t clk = userInputManager.readValidatedPinNumber("CC1101 CLK pin", state.getCC1101CLKPin(), forbidden);
    state.setCC1101CLKPin(clk);

    uint8_t miso = userInputManager.readValidatedPinNumber("CC1101 MISO pin", state.getCC1101MISOPin(), forbidden);
    state.setCC1101MISOPin(miso);

    uint8_t mosi = userInputManager.readValidatedPinNumber("CC1101 MOSI pin", state.getCC1101MOSIPin(), forbidden);
    state.setCC1101MOSIPin(mosi);

    uint8_t gdo0 = userInputManager.readValidatedPinNumber("CC1101 GDO0 pin", state.getCC1101GDO0Pin(), forbidden);
    state.setCC1101GDO0Pin(gdo0);

    uint8_t gdo2 = userInputManager.readValidatedPinNumber("CC1101 GDO2 pin", state.getCC1101GDO2Pin(), forbidden);
    state.setCC1101GDO2Pin(gdo2);

    terminalView.println("CC1101 Configured.");

    cc1101Service.configure(
        state.getCC1101MOSIPin(),
        state.getCC1101MISOPin(),
        state.getCC1101CLKPin(),
        state.getCC1101CSPin(),
        state.getCC1101GDO0Pin(),
        state.getCC1101GDO2Pin(),
        1000000
    );
}

/*
Help
*/
void CC1101Controller::handleHelp() {
    terminalView.println("");
    terminalView.println("Unknown CC1101 command. Usage:");
    terminalView.println("  send <data> - Send data via CC1101");
    terminalView.println("  config - Configure CC1101 pins");
    terminalView.println("  raw instructions, e.g: [0x9F r:3]");
    terminalView.println("");
}

/*
Ensure Configuration
*/
void CC1101Controller::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }

    cc1101Service.configure(
    state.getCC1101MOSIPin(),
    state.getCC1101MISOPin(),
    state.getCC1101CLKPin(),
    state.getCC1101CSPin(),
    state.getCC1101GDO0Pin(),
    state.getCC1101GDO2Pin(),
    1000000
    );
}