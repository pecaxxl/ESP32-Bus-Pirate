#ifndef DEVICE_M5STICK 

#include "UsbS3Controller.h"

/*
Constructor
*/
UsbS3Controller::UsbS3Controller(
    ITerminalView& terminalView,
    IInput& terminalInput,
    IInput& deviceInput,
    IUsbService& usbService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceInput(deviceInput),
      usbService(usbService),
      argTransformer(argTransformer),
      userInputManager(userInputManager)
{}

/*
Entry point for command
*/
void UsbS3Controller::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "stick") handleUsbStick();
    else if (cmd.getRoot() == "keyboard") handleKeyboard(cmd);
    else if (cmd.getRoot() == "mouse") handleMouse(cmd);
    else if (cmd.getRoot() == "gamepad") handleGamepad(cmd);
    else if (cmd.getRoot() == "reset") handleReset();
    else if (cmd.getRoot() == "config") handleConfig();
    else handleHelp();
}

/*
Keyboard
*/
void UsbS3Controller::handleKeyboard(const TerminalCommand& cmd) {
    auto sub = cmd.getSubcommand();

    if (sub.empty()) handleKeyboardBridge();
    else if (sub == "bridge") handleKeyboardBridge();
    else handleKeyboardSend(cmd);
}

/*
Keyboard Send
*/
void UsbS3Controller::handleKeyboardSend(const TerminalCommand& cmd) {
    terminalView.println("USB Keyboard: Configuring...");
    usbService.keyboardBegin();
    terminalView.println("USB Keyboard: Initialize...");
    auto full = cmd.getArgs().empty() ? cmd.getSubcommand() : cmd.getSubcommand() + " " + cmd.getArgs();
    usbService.keyboardSendString(full);
    // usbService.reset();
    terminalView.println("USB Keyboard: String sent.");
}

/*
Keyboard Bridge
*/
void UsbS3Controller::handleKeyboardBridge() {
    terminalView.println("USB Keyboard Bridge: Sending all keys to USB HID.");
    usbService.keyboardBegin();

    terminalView.println("\n[WARNING] If the USB device is plugged on the same host as");
    terminalView.println("          the terminal, it may cause looping issues with ENTER.");
    terminalView.println("          (That makes no sense to bridge your keyboard on the same host)\n");


    auto sameHost = userInputManager.readYesNo("Are you connected on the same host? (y/n)", true);

    if (sameHost) {
        terminalView.println("Same host, ENTER key will not be sent to USB HID.");
    }

    terminalView.println("USB Keyboard: Bridge started.. Press [ANY ESP32 BUTTON] to stop.");


    while (true) {
        // Esp32 button to stop
        char k = deviceInput.readChar();
        if (k != KEY_NONE) {
            terminalView.println("\r\nUSB Keyboard Bridge: Stopped by user.");
            break;
        }
        
        // Terminal to keyboard hid
        char c = terminalInput.readChar();

        // if we send \n in the terminal web browser
        // and the usb hid are on the same host
        // then it will loop infinitely
        if (c != KEY_NONE) { 
            if (c == '\n' && sameHost) continue;
            usbService.keyboardSendString(std::string(1, c));
            delay(20); // slow down looping
        }
    }
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

    if (cmd.getSubcommand() == "click") handleMouseClick();
    else if (cmd.getSubcommand() == "jiggle") handleMouseJiggle(cmd);
    else handleMouseMove(cmd);
}

/*
Mouse Jiggle
*/
void UsbS3Controller::handleMouseJiggle(const TerminalCommand& cmd) {
    int intervalMs = 1000; // defaut

    if (!cmd.getArgs().empty() && argTransformer.isValidNumber(cmd.getArgs())) {
        auto intervalMs = argTransformer.parseHexOrDec32(cmd.getArgs());
    }

    terminalView.println("USB Mouse: Jiggle started (" + std::to_string(intervalMs) + " ms)... Press [ENTER] to stop.");

    while (true) {
        // Random moves
        int dx = (int)random(-127, 127);
        int dy = (int)random(-127, 127);
        if (dx == 0 && dy == 0) dx = 1;

        usbService.mouseMove(dx, dy);

        // wait interval while listening for ENTER
        unsigned long t0 = millis();
        while ((millis() - t0) < intervalMs) {
            auto c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("USB Mouse: Jiggle stopped.\n");
                return;
            }
            delay(10);
        }
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
    terminalView.println("  keyboard");
    terminalView.println("  keyboard <text>");
    terminalView.println("  mouse <x> <y>");
    terminalView.println("  mouse click");
    terminalView.println("  mouse jiggle [ms]");
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