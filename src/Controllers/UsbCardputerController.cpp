#ifdef DEVICE_CARDPUTER

#include "UsbCardputerController.h"

/*
Constructor
*/
UsbCardputerController::UsbCardputerController(ITerminalView& terminalView, IInput& terminalInput, 
                             IUsbService& usbService, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), usbService(usbService), argTransformer(argTransformer), userInputManager(userInputManager) {}

/*
Entry point for command
*/
void UsbCardputerController::handleCommand(const TerminalCommand& cmd) {
    
    if (cmd.getRoot() == "stick") {
        handleUsbStick();
    } 
    
    else if (cmd.getRoot() == "keyboard") {
        handleKeyboardSend(cmd);
    } 

    else if (cmd.getRoot() == "mouse") {
        handleMouse(cmd);
    } 

    else if (cmd.getRoot() == "gamepad") {
        handleGamepad(cmd);
    } 

    else if (cmd.getRoot() == "reset") {
        handleReset();
    } 

    else if (cmd.getRoot() == "config") {
        handleConfig();
    } 
    
    else {
        handleHelp();
    }
}

/*
Keyboard Send
*/
void UsbCardputerController::handleKeyboardSend(const TerminalCommand& cmd) {
    terminalView.println("USB Keyboard: Configuring...");
    usbService.keyboardBegin();
    terminalView.println("USB Keyboard: Initialize...");
    usbService.keyboardSendString(cmd.getSubcommand());
    // usbService.reset();
    terminalView.println("USB Keyboard: String sent.");
}

/*
Mouse Move
*/
void UsbCardputerController::handleMouseMove(const TerminalCommand& cmd) {
    int x, y = 0;

    if (cmd.getSubcommand() == "move") {
        auto args = argTransformer.splitArgs(cmd.getArgs());
        x = argTransformer.parseHexOrDec(args[0]);
        y = argTransformer.parseHexOrDec(args[1]);;
    } else {
        x = argTransformer.parseHexOrDec(cmd.getSubcommand());
        y = argTransformer.parseHexOrDec(cmd.getArgs());
    }

    usbService.mouseMove(x, y);
    terminalView.println("USB Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

/*
Mouse Click
*/
void UsbCardputerController::handleMouseClick() {
    // Left click
    usbService.mouseClick(1);
    delay(100);
    usbService.mouseRelease(1);
    terminalView.println("USB Mouse: Click sent.");
}

/*
Mouse
*/
void UsbCardputerController::handleMouse(const TerminalCommand& cmd)  {
    terminalView.println("USB Mouse: Configuring HID...");
    usbService.mouseBegin();
    terminalView.println("USB Mouse: Initialize HID...");

    if (cmd.getSubcommand() == "click") {
        handleMouseClick();
    } else {
        handleMouseMove(cmd);
    }
}

/*
Gamepad
*/
void UsbCardputerController::handleGamepad(const TerminalCommand& cmd)  {
    terminalView.println("USB Gamepad: Configuring HID...");
    usbService.gamepadBegin();
    terminalView.println("USB Gamepad: Initialize HID...");
    usbService.gamepadPress(cmd.getSubcommand());
    terminalView.println("USB Gamepad: Key sent.");
} 

/*
Stick
*/
void UsbCardputerController::handleUsbStick() {
    terminalView.println("USB Stick: Starting... USB Drive can take 30sec to appear");
    usbService.storageBegin(state.getSpiCSPin(), state.getSpiCLKPin(), 
                            state.getSpiMISOPin(), state.getSpiMOSIPin());
}

/*
Config
*/
void UsbCardputerController::handleConfig() {
    terminalView.println("");
    terminalView.println("USB Configuration:");

    GlobalState& state = GlobalState::getInstance();

    uint8_t cs = userInputManager.readValidatedUint8("SD Card CS pin", state.getSpiCSPin());
    state.setSpiCSPin(cs);

    uint8_t clk = userInputManager.readValidatedUint8("SD Card CLK pin", state.getSpiCLKPin());
    state.setSpiCLKPin(clk);

    uint8_t miso = userInputManager.readValidatedUint8("SD Card MISO pin", state.getSpiMISOPin());
    state.setSpiMISOPin(miso);

    uint8_t mosi = userInputManager.readValidatedUint8("SD Card MOSI pin", state.getSpiMOSIPin());
    state.setSpiMOSIPin(mosi);

    terminalView.println("USB Configured.");
    terminalView.println("\n[WARNING] If you're using USB Serial terminal mode,");
    terminalView.println("          using USB commands may interrupt the session.");
    terminalView.println("          Use Web UI or restart if connection is lost.\n");
}


/*
Reset
*/
void UsbCardputerController::handleReset() {
    usbService.reset();
    terminalView.println("USB Reset: Resetting...");
}

/*
Help
*/
void UsbCardputerController::handleHelp() {
    terminalView.println("Unknown command.");
    terminalView.println("Usage:");
    terminalView.println("  stick");
    terminalView.println("  keyboard <text>");
    terminalView.println("  mouse <x> <y>");
    terminalView.println("  mouse click");
    terminalView.println("  gamepad <key>, eg. A, B, LEFT...");
    terminalView.println("  reset");
    terminalView.println("  config");
}

/*
Ensure Configuration
*/
void UsbCardputerController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}

#endif