#include "Controllers/DioController.h"
#include <sstream>
#include <algorithm>

/*
Constructor
*/
DioController::DioController(ITerminalView& terminalView, IInput& terminalInput, PinService& pinService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), pinService(pinService), argTransformer(argTransformer) {}

/*
Entry point to handle a DIO command
*/
void DioController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "sniff") {
        handleSniff(cmd);
    }

    else if (cmd.getRoot() == "read") {
        handleReadPin(cmd);
    } 

    else if (cmd.getRoot() == "set") {
        handleSetPin(cmd);
    } 

    else if (cmd.getRoot() == "pullup") {
        handlePullup(cmd);
    }

    else if (cmd.getRoot() == "pwm") {
        handlePwm(cmd);
    }

    else if (cmd.getRoot() == "reset") {
        handleResetPin(cmd);
    }
        
    else {
        handleHelp();
    }
}

/*
Read
*/
void DioController::handleReadPin(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: read <pin>");
        return;
    }

    int pin = std::stoi(cmd.getSubcommand());
    int value = pinService.read(pin);
    terminalView.println("Pin " + std::to_string(pin) + " = " + std::to_string(value));
}

/*
Set
*/
void DioController::handleSetPin(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: set <pin> <IN/OUT/HI/LOW>");
        return;
    }

    int pin = std::stoi(cmd.getSubcommand());
    std::string arg = cmd.getArgs();
    if (arg.empty()) {
        terminalView.println("DIO Set: Invalid argument.");
        return;
    }

    char c = std::toupper(arg[0]);

    switch (c) {
        case 'I':
            pinService.setInput(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to INPUT");
            break;
        case 'O':
            pinService.setOutput(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to OUTPUT");
            break;
        case 'H':
            pinService.setOutput(pin);
            pinService.setHigh(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to HIGH");
            break;
        case 'L':
            pinService.setOutput(pin);
            pinService.setLow(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to LOW");
            break;
        default:
            terminalView.println("Unknown command. Use I, O, H, or L.");
            break;
    }
}


/*
Pullup
*/
void DioController::handlePullup(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: pullup <pin>");
        return;
    }

    int pin = std::stoi(cmd.getSubcommand());
    pinService.setInputPullup(pin);

    terminalView.println("DIO Pullup: Set on pin " + std::to_string(pin));
}

/*
Sniff
*/
void DioController::handleSniff(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: sniff <pin>");
        return;
    }

    int pin = std::stoi(cmd.getSubcommand());
    pinService.setInput(pin);

    terminalView.println("DIO Sniff: Pin " + std::to_string(pin) + "... Press [ENTER] to stop");
    
    int last = pinService.read(pin);
    terminalView.println("Initial state: " + std::to_string(last));

    unsigned long lastCheck = millis();
    while (true) {
        // check ENTER press
        if (millis() - lastCheck > 10) {
            lastCheck = millis();
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("DIO Sniff: Stopped.");
                break;
            }
        }

        // check pin state
        int current = pinService.read(pin);
        if (current != last) {
            std::string transition = (last == 0 && current == 1)
                ? "LOW  -> HIGH"
                : "HIGH -> LOW";
            terminalView.println("Pin " + std::to_string(pin) + ": " + transition);
            last = current;
        }
    }
}

/*
Pwm
*/
void DioController::handlePwm(const TerminalCommand& cmd) {
    auto sub = cmd.getSubcommand();
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (!sub.empty() && args.size() != 2) {
        terminalView.println("DIO PWN: Invalid syntax. Use:");
        terminalView.println("  pwm <pin> <frequency> <duty>");
        return;
    }

    if (!argTransformer.isValidNumber(sub) ||
        !argTransformer.isValidNumber(args[0]) ||
        !argTransformer.isValidNumber(args[1])) {
        terminalView.println("DIO PWN: All arguments must be valid numbers.");
        return;
    }

    uint8_t pin = argTransformer.toUint8(sub);
    uint32_t freq = argTransformer.toUint32(args[0]);
    uint8_t duty = argTransformer.toUint8(args[1]);

    if (duty > 100) {
        terminalView.println("DIO PWN: Duty cycle must be between 0 and 100.");
        return;
    }

    int channel = pin % 16;
    int resolution = 8;

    ledcSetup(channel, freq, resolution);
    ledcAttachPin(pin, channel);
    uint32_t dutyVal = (duty * ((1 << resolution) - 1)) / 100;
    ledcWrite(channel, dutyVal);

    terminalView.println("DIO PWM: Pin " + std::to_string(pin) +
                         " (" + std::to_string(freq) + "Hz, " +
                         std::to_string(duty) + "% duty).");
}

/*
Reset
*/
void DioController::handleResetPin(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: reset <pin>");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("DIO Reset: Invalid pin number.");
        return;
    }

    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());

    // Detacher PWM
    ledcDetachPin(pin);

    // Reset Pullup
    pinService.setInput(pin);

    terminalView.println("DIO Reset: Pin " + std::to_string(pin) + " to INPUT (no pull-up, no PWM).");
}

/*
Help
*/
void DioController::handleHelp() {
    terminalView.println("Unknown DIO command. Usage:");
    terminalView.println("  sniff <pin>");
    terminalView.println("  read <pin>");
    terminalView.println("  set <pin> <H/L/I/O>");
    terminalView.println("  pullup <pin>");
    terminalView.println("  pwm <pin> <freq> <duty>");
    terminalView.println("  reset <pin>");
}