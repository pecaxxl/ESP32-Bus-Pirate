#include "Controllers/UtilityController.h"

/*
Constructor
*/
UtilityController::UtilityController(ITerminalView& terminalView, IInput& terminalInput, PinService& pinService, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), pinService(pinService), userInputManager(userInputManager) {}

/*
Entry point for command
*/
void UtilityController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "help" || cmd.getRoot() == "h" || cmd.getRoot() == "?") {
        handleHelp();
    }

    else if (cmd.getRoot() == "P") {
        handleEnablePullups();
    }

    else if (cmd.getRoot() == "p") {
        handleDisablePullups();
    }

    else {
        terminalView.println("Unknown command. Try 'help'.");
    }
}

/*
Mode Change
*/
ModeEnum UtilityController::handleModeChangeCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() != "mode" && cmd.getRoot() != "m") {
        terminalView.println("Invalid command for mode change.");
        return ModeEnum::None;
    }

    if (!cmd.getSubcommand().empty()) {
        ModeEnum newMode = ModeEnumMapper::fromString(cmd.getSubcommand());
        if (newMode != ModeEnum::None) {
            terminalView.println("Mode changed to " + ModeEnumMapper::toString(newMode));
            terminalView.println(""); 
            return newMode;
        } else {
            terminalView.println("Unknown mode: " + cmd.getSubcommand());
            return ModeEnum::None;
        }
    }

    return handleModeSelect();
}

/*
Mode Select
*/
ModeEnum UtilityController::handleModeSelect() {
    terminalView.println("");
    terminalView.println("Select mode:");
    std::vector<ModeEnum> modes;

    for (int i = 0; i <= static_cast<int>(ModeEnum::JTAG); ++i) {
        ModeEnum mode = static_cast<ModeEnum>(i);
        std::string name = ModeEnumMapper::toString(mode);
        if (!name.empty()) {
            modes.push_back(mode);
            terminalView.println("  " + std::to_string(modes.size()) + ". " + name);
        }
    }

    terminalView.println("");
    terminalView.print("Mode Number > ");
    auto modeNumber = userInputManager.readModeNumber();

    if (modeNumber == -1) {
        terminalView.println("");
        terminalView.println("");
        terminalView.println("Invalid input.");
        return ModeEnum::None;
    } else if (modeNumber >= 1 && modeNumber <= modes.size()) {
        ModeEnum selected = modes[modeNumber - 1];
        if (static_cast<int>(selected) > 9) {
            terminalView.println(""); // Hack to render correctly on web terminal
        }
        terminalView.println("");
        terminalView.println("Mode changed to " + ModeEnumMapper::toString(selected));
        terminalView.println("");
        return selected;
    } else {
        terminalView.println("");
        terminalView.println("Invalid mode number.");
        terminalView.println("");
        return ModeEnum::None;
    }
}

/*
Pullup: p
*/
void UtilityController::handleDisablePullups() {
    auto mode = state.getCurrentMode();
    switch (mode) {
        case ModeEnum::SPI:
            pinService.setInput(state.getSdCardMISOPin());
            terminalView.println("SPI: Pull-ups disabled on MISO");
            break;

        case ModeEnum::I2C:
            pinService.setInput(state.getI2cSdaPin());
            pinService.setInput(state.getI2cSclPin());
            terminalView.println("I2C: Pull-ups disabled on SDA, SCL.");
            break;

        case ModeEnum::OneWire:
            pinService.setInput(state.getOneWirePin());
            terminalView.println("1-Wire: Pull-up disabled on DQ.");
            break;

        case ModeEnum::UART:
            pinService.setInput(state.getUartRxPin());
            terminalView.println("UART: Pull-ups disabled on RX.");
            break;

        case ModeEnum::HDUART:
            pinService.setInput(state.getHdUartPin());
            terminalView.println("HDUART: Pull-up disabled on IO pin.");
            break;

        default:
            terminalView.println("Pull-ups not applicable for this mode.");
            break;
    }
}

/*
Pullup P
*/
void UtilityController::handleEnablePullups() {
    auto mode = state.getCurrentMode();
    switch (mode) {
        case ModeEnum::SPI:
            pinService.setInput(state.getSdCardMISOPin());
            pinService.setInputPullup(state.getSdCardMISOPin());
            terminalView.println("SPI: Pull-up enabled on MISO.");
            break;

        case ModeEnum::I2C:
            pinService.setInputPullup(state.getI2cSdaPin());
            pinService.setInputPullup(state.getI2cSclPin());
            terminalView.println("I2C: Pull-ups enabled on SDA, SCL.");
            break;

        case ModeEnum::OneWire:
            pinService.setInputPullup(state.getOneWirePin());
            terminalView.println("1-Wire: Pull-up enabled on DQ.");
            break;

        case ModeEnum::UART:
            pinService.setInputPullup(state.getUartRxPin());
            terminalView.println("UART: Pull-up enabled on RX.");
            break;

        case ModeEnum::HDUART:
            pinService.setInputPullup(state.getHdUartPin());
            terminalView.println("HDUART: Pull-up enabled on IO pin.");
            break;

        default:
            terminalView.println("Pull-ups not applicable for this mode.");
            break;
    }
}

/*
Help
*/
void UtilityController::handleHelp() {
    terminalView.println("");
    terminalView.println("       +=== Help: Available Commands ===+");
    terminalView.println("");

    terminalView.println(" General:");
    terminalView.println("  help                 - Show this help message");
    terminalView.println("  mode <name>          - Change or select active mode");
    terminalView.println("  P                    - Enable pull-up resistors");
    terminalView.println("  p                    - Disable pull-up resistors");
    
    terminalView.println("");
    terminalView.println(" 1. HiZ:");
    terminalView.println("  (default mode)       - All interfaces disabled");

    terminalView.println("");
    terminalView.println(" 2. 1WIRE:");
    terminalView.println("  scan                 - Scan for 1-Wire devices");
    terminalView.println("  ping                 - Ping for 1-Wire device");
    terminalView.println("  sniff                - Sniff for 1-Wire raw bytes traffic");
    terminalView.println("  read                 - Read ID and scratchpad");
    terminalView.println("  write id <8 bytes>   - Write the ibutton ID");
    terminalView.println("  write sp <8 bytes>   - Write scratchpad (No ID Check)");
    terminalView.println("  config               - Configure 1WIRE settings");
    terminalView.println("  [0xAA r:8] ...       - Instruction syntax supported");

    terminalView.println("");
    terminalView.println(" 3. UART:");
    terminalView.println("  scan                 - Try to detect UART baudrate automatically");
    terminalView.println("  ping                 - Send commands to UART device for a response");
    terminalView.println("  read                 - Read data from UART device at current baud");
    terminalView.println("  write <text>         - Send data to UART device at current baud");
    terminalView.println("  bridge               - Enable UART bridge mode (Read/Write)");
    terminalView.println("  glitch               - UART timing voltage glitcher [NYI]");
    terminalView.println("  config               - Configure UART settings");
    terminalView.println("  ['Hello] [r:64]...   - Instruction syntax supported");
 
    terminalView.println("");
    terminalView.println(" 4. HDUART:");
    terminalView.println("  bridge               - Open HDUART bridge mode (Read/Write)");
    terminalView.println("  config               - Configure HDUART settings");
    terminalView.println("  [0x1 D:10 r:255]     - Instruction syntax supported");

    terminalView.println("");
    terminalView.println(" 5. I2C:");
    terminalView.println("  scan                 - Scan for I2C devices");
    terminalView.println("  ping <addr>          - Ping an I2C address (check ACK)");
    terminalView.println("  sniff                - Sniff for I2C raw bytes traffic");
    terminalView.println("  read <addr> <reg>    - Read 1 byte from a register");
    terminalView.println("  write <addr reg val> - Write 1 byte to a register");
    terminalView.println("  config               - Configure I2C settings");
    terminalView.println("  [0x13 0x4B 0x1] ...  - Instruction syntax supported");

    terminalView.println("");
    terminalView.println(" 6. SPI:");
    terminalView.println("  sniff                - Sniff for SPI raw bytes traffic [NYI]");
    terminalView.println("  sdcard               - Perform operations on SD cards [NYI]");
    terminalView.println("  flash probe          - Identify SPI chip [NYI]");
    terminalView.println("  flash read           - Read the content of SPI flash [NYI]");
    terminalView.println("  flash write          - Write to SPI flash [NYI]");
    terminalView.println("  flash erase          - Erase SPI flash [NYI]");
    terminalView.println("  config               - Configure SPI settings [NYI]");
    terminalView.println("  [..]                 - Instruction syntax supported [NYI]");

    terminalView.println("");
    terminalView.println(" 7. 2WIRE:");
    terminalView.println("  sniff                - Sniff for 2WIRE raw bytes traffic [NYI]");
    terminalView.println("  [..]                 - Instruction syntax supported [NYI]");

    terminalView.println("");
    terminalView.println(" 8. 3WIRE:");
    terminalView.println("  [..]                 - Instruction syntax supported [NYI]");

    terminalView.println("");
    terminalView.println(" 9. DIO:");
    terminalView.println("  sniff <pin>          - Sniff pin toggle states High/Low");
    terminalView.println("  read <pin>           - Get pin state High/Low");
    terminalView.println("  set <pin> <H/L/I/O>  - Set pin state High/Low, Input/Output");
    terminalView.println("  pullup <pin>         - Set pin pullup resistor");
    terminalView.println("  pwm <pin> freq <dut> - Set PWM (freq Hz, duty %) on pin");
    terminalView.println("  reset <pin>          - Reset pin to default mode");

    terminalView.println("");
    terminalView.println(" 10. LED:");
    terminalView.println("  write i <r> <g> <b>  - Set color of LED i (RGB, 0-255) [NYI]");
    terminalView.println("  setprotocol          - Select LED type (WS2801, APA102, SK6812...) [NYI]");
    terminalView.println("  test                 - Animate test sequence on LEDs [NYI]");
    terminalView.println("  config               - Configure LED settings [NYI]");
    terminalView.println("  reset                - Turn off all LEDs (0,0,0) [NYI]");

    terminalView.println("");
    terminalView.println(" 11. INFRARED:");
    terminalView.println("  send <dev> sub <cmd> - Send IR signal");
    terminalView.println("  receive              - Receive and decode IR signal");
    terminalView.println("  setprotocol          - Set IR protocol type (for send)");
    terminalView.println("  devicebgone          - Blasting OFF for TVs, projectors, boxes...");
    terminalView.println("  config               - Configure IR settings");

    terminalView.println("");
    terminalView.println(" 12. USB:");
    terminalView.println("  stick                - Mount SD card as a USB stick");
    terminalView.println("  keyboard <text>      - Simulate keyboard and type <text>");
    terminalView.println("  mouse <x> <y>        - Simulate mouse move by <x>, <y> pixels");
    terminalView.println("  mouse click          - Simulate mouse left click");
    terminalView.println("  gamepad <key>        - Simulate a gamepad press (A, B, UP...)");
    terminalView.println("  reset                - Reset the USB interface");
    terminalView.println("  config               - Configure USB settings");

    terminalView.println("");
    terminalView.println(" 13. BLUETOOTH:");
    terminalView.println("  scan                 - Scan for nearby devices and explore services");
    terminalView.println("  pair <addr>          - Pair with a device and explore services");
    terminalView.println("  sniff                - Sniff Bluetooth data on server [NYI]");
    terminalView.println("  spoof <addr>         - Set the Bluetooth mac address (before init)");
    terminalView.println("  status               - Show current Bluetooth status");
    terminalView.println("  server               - Create an HID server and start advertising");
    terminalView.println("  keyboard <text>      - Emulate a Bluetooth keyboard and send <text>");
    terminalView.println("  mouse <x> <y>        - Emulate a Bluetooth mouse move");
    terminalView.println("  mouse click          - Emulate Blutooth mouse left click");
    terminalView.println("  reset                - Reset the Bluetooth interface");

    terminalView.println("");
    terminalView.println(" 14. WIFI:");
    terminalView.println("  scan                 - List nearby Wi-Fi networks");
    terminalView.println("  ping <host>          - Ping a remote host");
    terminalView.println("  sniff                - Monitor nearby Wi-Fi packets [NYI]");
    terminalView.println("  connect <ssid> <pw>  - Connect to a Wi-Fi network");
    terminalView.println("  status               - Show current Wi-Fi status");
    terminalView.println("  disconnect           - Disconnect from Wi-Fi");
    terminalView.println("  ap <ssid> <password> - Set a Wi-Fi access point");
    terminalView.println("  webui                - Show the web UI IP");
    terminalView.println("  reset                - Reset Wi-Fi interface");

    terminalView.println("");
    terminalView.println(" 15. JTAG:");
    terminalView.println("  scan                 - Scan for JTAG/SWD pinouts [NYI]");

    terminalView.println("");
    terminalView.println(" Instructions (available in most modes):");
    terminalView.println(" See documentation for instruction syntax.");
    terminalView.println("");
    terminalView.println(" Note: Use 'mode' to switch between I2C, UART, SPI, etc.");
    terminalView.println("");
}

bool UtilityController::isGlobalCommand(const TerminalCommand& cmd) {
    std::string root = cmd.getRoot();
    return (root == "help" || root == "h" || root == "?" ||
            root == "mode" || root == "m" || root == "l"
                           || root == "P" || root == "p");
}