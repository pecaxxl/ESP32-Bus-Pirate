#include "InfraredController.h"

InfraredController::InfraredController(ITerminalView& view, IInput& terminalInput, InfraredService& service, ArgTransformer& argTransformer, UserInputManager& userInputManager)
    : terminalView(view), terminalInput(terminalInput), infraredService(service), argTransformer(argTransformer), userInputManager(userInputManager) {}

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

    for (int i = 0; i < 3; ++i) {
        infraredService.sendInfraredCommand(infraredCommand);
        delay(150);
    }

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
        }
    }
}

/* 
DeviceBgone
*/
void InfraredController::handleDeviceBgone() {
    terminalView.println("Sending Device-B-Gone commands... Press ENTER to stop");

    for (const auto& cmd : deviceBgoneCommands) {

        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("Infrared Devicebgone: Interrupted by user.");
            return;
        }

        for (int i = 0; i < 2; ++i) { // send 2x per command
            infraredService.sendInfraredCommand(cmd);
            delay(150);
        }

        terminalView.println(
            "Sent On/Off to protocol=" + InfraredProtocolMapper::toString(cmd.getProtocol()) +
            " device=" + std::to_string(cmd.getDevice()) +
            " sub=" + std::to_string(cmd.getSubdevice()) +
            " cmd=" + std::to_string(cmd.getFunction())
        );
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
    terminalView.println("\nInfrared Configuration:");

    uint8_t txPin = userInputManager.readValidatedUint8("Infrared TX pin", state.getInfraredTxPin());
    uint8_t rxPin = userInputManager.readValidatedUint8("Infrared RX pin", state.getInfraredRxPin());

    state.setInfraredTxPin(txPin);
    state.setInfraredRxPin(rxPin);
    infraredService.configure(txPin, rxPin);

    terminalView.println("Infrared configured.\n");
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

void InfraredController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}