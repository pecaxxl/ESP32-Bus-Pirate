#ifndef DEVICE_M5STICK 

#include "UsbS3Controller.h"

/*
Constructor
*/
UsbS3Controller::UsbS3Controller(ITerminalView& terminalView, IInput& terminalInput, 
                             IUsbService& usbService, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), usbService(usbService), argTransformer(argTransformer), userInputManager(userInputManager) {}

/*
Entry point for command
*/
void UsbS3Controller::handleCommand(const TerminalCommand& cmd) {
    
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
void UsbS3Controller::handleKeyboardSend(const TerminalCommand& cmd) {
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
void UsbS3Controller::handleMouseMove(const TerminalCommand& cmd) {
    int x, y = 0;

    // mouse move x y
    if (cmd.getSubcommand() == "move") {
        auto args = argTransformer.splitArgs(cmd.getArgs());
        x = argTransformer.toClampedInt8(args[0]);
        y = argTransformer.toClampedInt8(args[1]);
    // mouse x y
    } else {
        x = argTransformer.toClampedInt8(cmd.getSubcommand());
        y = argTransformer.toClampedInt8(cmd.getArgs());
    }

    usbService.mouseMove(x, y);
    terminalView.println("USB Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

/*
Mouse Click
*/
void UsbS3Controller::handleMouseClick() {
    // Left click
    usbService.mouseClick(1);
    delay(100);
    usbService.mouseRelease(1);
    terminalView.println("USB Mouse: Click sent.");
}

/*
Mouse
*/
void UsbS3Controller::handleMouse(const TerminalCommand& cmd)  {
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
void UsbS3Controller::handleGamepad(const TerminalCommand& cmd) {
    terminalView.println("USB Gamepad: Configuring HID...");
    usbService.gamepadBegin();

    std::string subcmd = cmd.getSubcommand();
    std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);

    if (subcmd == "up" || subcmd == "down" || subcmd == "left" || subcmd == "right" ||
        subcmd == "a" || subcmd == "b") {
        
        usbService.gamepadPress(subcmd);
        terminalView.println("USB Gamepad: Key sent.");

    } else {
        terminalView.println("USB Gamepad: Unknown input. Try up, down, left, right, a, b");
    }
}

/*
Stick
*/
void UsbS3Controller::handleUsbStick() {
    terminalView.println("USB Stick: Starting... USB Drive can take 30sec to appear");
    usbService.storageBegin(state.getSpiCSPin(), state.getSpiCLKPin(), 
                            state.getSpiMISOPin(), state.getSpiMOSIPin());
}

/*
Config
*/
void UsbS3Controller::handleConfig() {
    terminalView.println("");
    terminalView.println("USB Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t cs = userInputManager.readValidatedPinNumber("SD Card CS pin", state.getSpiCSPin(), forbidden);
    state.setSpiCSPin(cs);

    uint8_t clk = userInputManager.readValidatedPinNumber("SD Card CLK pin", state.getSpiCLKPin(), forbidden);
    state.setSpiCLKPin(clk);

    uint8_t miso = userInputManager.readValidatedPinNumber("SD Card MISO pin", state.getSpiMISOPin(), forbidden);
    state.setSpiMISOPin(miso);

    uint8_t mosi = userInputManager.readValidatedPinNumber("SD Card MOSI pin", state.getSpiMOSIPin(), forbidden);
    state.setSpiMOSIPin(mosi);

    terminalView.println("USB Configured.");
    terminalView.println("\n[WARNING] If you're using USB Serial terminal mode,");
    terminalView.println("          using USB commands may interrupt the session.");
    terminalView.println("          Use Web UI or restart if connection is lost.\n");
}


/*
Reset
*/
void UsbS3Controller::handleReset() {
    usbService.reset();
    terminalView.println("USB Reset: Disable interfaces...");
}

/*
Help
*/
void UsbS3Controller::handleHelp() {
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
void UsbS3Controller::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}

#endif