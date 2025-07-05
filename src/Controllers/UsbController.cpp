#include "UsbController.h"

/*
Constructor
*/
UsbController::UsbController(ITerminalView& terminalView, IInput& terminalInput, UsbService& usbService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), usbService(usbService), argTransformer(argTransformer) {}

/*
Entry point for command
*/
void UsbController::handleCommand(const TerminalCommand& cmd) {
    
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
void UsbController::handleKeyboardSend(const TerminalCommand& cmd) {
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
void UsbController::handleMouseMove(const TerminalCommand& cmd) {
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
void UsbController::handleMouseClick() {
    // Left click
    usbService.mouseClick(1);
    delay(100);
    usbService.mouseRelease(1);
}

/*
Mouse
*/
void UsbController::handleMouse(const TerminalCommand& cmd)  {
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
void UsbController::handleGamepad(const TerminalCommand& cmd)  {
    terminalView.println("USB Gamepad: Configuring HID...");
    usbService.gamepadBegin();
    terminalView.println("USB Gamepad: Initialize HID...");
    usbService.gamepadPress(cmd.getSubcommand());
    terminalView.println("USB Gamepad: Key sent.");
} 

/*
Stick
*/
void UsbController::handleUsbStick() {
    usbService.storageBegin(state.getSdCardCSPin(), state.getSdCardCLKPin(), 
                            state.getSdCardMISOPin(), state.getSdCardMOSIPin());
}

/*
Config
*/
void UsbController::handleConfig() {
    terminalView.println("");
    terminalView.println("USB Configuration:");

    terminalView.print("SD Card CS pin [" + std::to_string(state.getSdCardCSPin()) + "]: ");
    std::string csInput = getUserInput();
    uint8_t cs = csInput.empty() ? state.getSdCardCSPin() : static_cast<uint8_t>(std::stoi(csInput));
    state.setSdCardCSPin(cs);

    terminalView.print("SD Card CLK pin [" + std::to_string(state.getSdCardCLKPin()) + "]: ");
    std::string clkInput = getUserInput();
    uint8_t clk = clkInput.empty() ? state.getSdCardCLKPin() : static_cast<uint8_t>(std::stoi(clkInput));
    state.setSdCardCLKPin(clk);

    terminalView.print("SD Card MISO pin [" + std::to_string(state.getSdCardMISOPin()) + "]: ");
    std::string misoInput = getUserInput();
    uint8_t miso = misoInput.empty() ? state.getSdCardMISOPin() : static_cast<uint8_t>(std::stoi(misoInput));
    state.setSdCardMISOPin(miso);

    terminalView.print("SD Card MOSI pin [" + std::to_string(state.getSdCardMOSIPin()) + "]: ");
    std::string mosiInput = getUserInput();
    uint8_t mosi = mosiInput.empty() ? state.getSdCardMOSIPin() : static_cast<uint8_t>(std::stoi(mosiInput));
    state.setSdCardMOSIPin(mosi);

    terminalView.println("USB Configured.");

    terminalView.println("\n[WARNING] If you're using USB Serial terminal mode,");
    terminalView.println("          using USB commands may interrupt the session.");
    terminalView.println("          Use Web UI or restart if connection is lost.\n");
}

/*
Reset
*/
void UsbController::handleReset() {
    usbService.reset();
    terminalView.println("USB mode: Resetting");
}

/*
Help
*/
void UsbController::handleHelp() {
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

std::string UsbController::getUserInput() {
    std::string inputStr;
    while (true) {
        char c = terminalInput.handler();
        if (c == '\r' || c == '\n') {
            terminalView.println("");
            break;
        }
        inputStr += c;
        terminalView.print(std::string(1, c));
    }
    return inputStr;
}

void UsbController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
