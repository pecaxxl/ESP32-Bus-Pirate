#include "InfraredController.h"

InfraredController::InfraredController(ITerminalView& view, IInput& terminalInput, InfraredService& service, ArgTransformer& argTransformer)
    : terminalView(view), terminalInput(terminalInput), infraredService(service), argTransformer(argTransformer) {}

/*
Entry point to handle Infrared command
*/
void InfraredController::handleCommand(const TerminalCommand& command) {
    if (command.getRoot() == "config") {
        handleConfig();
    } 

    else if (command.getRoot() == "send") {
        handleSend(command);
    } 

    else if (command.getRoot() == "receive") {
        handleReceive();
    } 

    else if (command.getRoot() == "devicebgone") {
        handleDeviceBgone();
    }

    else if (command.getRoot() == "setprotocol") {
        handleSetProtocol();
    }

    else {
        handleHelp();
    }
}

/*
Send
*/
void InfraredController::handleSend(const TerminalCommand& command) {
    std::istringstream iss(command.getArgs());
    std::string subStr, cmdStr;
    iss >> subStr >> cmdStr;
    auto addrStr = command.getSubcommand();

    if (addrStr.empty() || subStr.empty() || cmdStr.empty()) {
        terminalView.println("Missing argument. Usage: send <device> <sub> <cmd>");
        return;
    }

    int device, subdevice, function;
    if (!argTransformer.parseInt(addrStr, device) ||
        !argTransformer.parseInt(subStr, subdevice) ||
        !argTransformer.parseInt(cmdStr, function)) {
        terminalView.println("Invalid number format. Use decimal or hex.");
        return;
    }

    InfraredCommand infraredCommand;
    infraredCommand.setDevice(device);
    infraredCommand.setSubdevice(subdevice);
    infraredCommand.setFunction(function);
    infraredCommand.setProtocol(state.getInfraredProtocol());

    infraredService.sendInfraredCommand(infraredCommand);

    terminalView.println("IR command sent with protocol " + InfraredProtocolMapper::toString(state.getInfraredProtocol()));
}

/*
Receive
*/
void InfraredController::handleReceive() {
    terminalView.println("Receiving infrared signal...");
    terminalView.println("Press ENTER to cancel.\n");

    while (true) {
        // Check Enter press
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("INFRARED Receive: Cancelled by user.");
            return;
        }

        // Try receive and decode signal
        InfraredCommand cmd = infraredService.receiveInfraredCommand();
        if (cmd.getProtocol() != RAW) {
            terminalView.println("");
            terminalView.println("Infrared signal received:");
            terminalView.println("  Protocol : " + InfraredProtocolMapper::toString(cmd.getProtocol()));
            terminalView.println("  Device   : " + std::to_string(cmd.getDevice()));
            terminalView.println("  SubDev   : " + std::to_string(cmd.getSubdevice()));
            terminalView.println("  Command  : " + std::to_string(cmd.getFunction()));
            terminalView.println("");
            terminalView.println("INFRARED Receive: Waiting for next signal or press ENTER to exit.");
            delay(200);
        }
    }
}

/* 
DeviceBgone
*/
void InfraredController::handleDeviceBgone() {
    terminalView.println("Sending Device-B-Gone commands...");

    for (const auto& cmd : deviceBgoneCommands) {
        infraredService.sendInfraredCommand(cmd);
        terminalView.println(
            "Sent On/Off to protocol=" + InfraredProtocolMapper::toString(cmd.getProtocol()) +
            " device=" + std::to_string(cmd.getDevice()) +
            " sub=" + std::to_string(cmd.getSubdevice()) +
            " cmd=" + std::to_string(cmd.getFunction())
        );
        delay(40); // small delay between signals
    }

    terminalView.println("Device-B-Gone sequence completed.");
}

/*
Set protocol
*/
void InfraredController::handleSetProtocol() {
    terminalView.println("");
    terminalView.println("Select Infrared Protocol:");

    std::vector<InfraredProtocolEnum> protocols;

    for (int i = 0; i <= static_cast<int>(RAW); ++i) {
        InfraredProtocolEnum proto = static_cast<InfraredProtocolEnum>(i);
        std::string name = InfraredProtocolMapper::toString(proto);

        // avoid double name
        if (!name.empty() && 
            std::find_if(protocols.begin(), protocols.end(),
                [proto](InfraredProtocolEnum e) { return InfraredProtocolMapper::toString(e) == InfraredProtocolMapper::toString(proto); }) == protocols.end()) {
            protocols.push_back(proto);
            terminalView.println("  " + std::to_string(protocols.size()) + ". " + name);
        }
    }

    terminalView.println("");
    terminalView.print("Protocol Number > ");

    std::string inputStr;
    while (true) {
        char c = terminalInput.handler();
        if (c == '\r' || c == '\n') {
            terminalView.println("");
            break;
        }

        if (std::isdigit(c)) {
            inputStr += c;
            terminalView.print(std::string(1, c));
        } else {
            terminalView.println("\nInvalid input: only digits allowed.");
            return;
        }
    }

    if (inputStr.empty()) {
        terminalView.println("No input given.");
        return;
    }

    int index = std::stoi(inputStr);
    if (index >= 1 && index <= static_cast<int>(protocols.size())) {
        InfraredProtocolEnum selected = protocols[index - 1];
        GlobalState::getInstance().setInfraredProtocol(selected);
        terminalView.println("Protocol changed to " + InfraredProtocolMapper::toString(selected));
    } else {
        terminalView.println("Invalid protocol number.");
    }
}

/*
Config
*/
void InfraredController::handleConfig() {
    terminalView.println("");
    terminalView.println("Infrared Configuration:");

    GlobalState& state = GlobalState::getInstance();
    uint8_t txPin = state.getInfraredTxPin();
    uint8_t rxPin = state.getInfraredRxPin();

    // TX
    while (true) {
        terminalView.print("Infrared TX pin [" + std::to_string(txPin) + "]: ");
        std::string txStr = getUserInput();

        if (txStr.empty()) break;
        if (argTransformer.isValidNumber(txStr)) {
            txPin = argTransformer.toUint8(txStr);
            break;
        } else {
            terminalView.println("Invalid pin value. Please enter a valid number.");
        }
    }

    // RX
    while (true) {
        terminalView.print("Infrared RX pin [" + std::to_string(rxPin) + "]: ");
        std::string rxStr = getUserInput();

        if (rxStr.empty()) break;
        if (argTransformer.isValidNumber(rxStr)) {
            rxPin = argTransformer.toUint8(rxStr);
            break;
        } else {
            terminalView.println("Invalid pin value. Please enter a valid number.");
        }
    }

    state.setInfraredTxPin(txPin);
    state.setInfraredRxPin(rxPin);
    infraredService.configure(txPin, rxPin);

    terminalView.println("Infrared configured.");
    terminalView.println("");
}

/*
Help
*/
void InfraredController::handleHelp() {
    terminalView.println("Unknown INFRARED command. Usage:");
    terminalView.println("  send <addr> <subadd> <cmd>");
    terminalView.println("  receive");
    terminalView.println("  setprotocol");
    terminalView.println("  devicebgone");
    terminalView.println("  config");
}

std::string InfraredController::getUserInput() {
    std::string result;
    while (true) {
        char c = terminalInput.handler();
        if (c == '\r' || c == '\n') break;
        result += c;
        terminalView.print(std::string(1, c));
    }
    terminalView.println("");
    return result;
}

void InfraredController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}