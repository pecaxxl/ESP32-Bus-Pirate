#include "ActionDispatcher.h"

/*
Constructor
*/
ActionDispatcher::ActionDispatcher(DependencyProvider& provider)
    : provider(provider) {}


/*
Setup
*/
void ActionDispatcher::setup(TerminalTypeEnum terminalType, std::string terminalInfos) {
    provider.getDeviceView().initialize();
    provider.getDeviceView().welcome(terminalType, terminalInfos);

    if (terminalType == TerminalTypeEnum::Serial) {
        provider.getTerminalView().initialize();
        provider.getTerminalView().waitPress();
        provider.getTerminalInput().waitPress();
        provider.getTerminalView().welcome(terminalType, terminalInfos);
    } else {
        provider.getTerminalView().initialize();
        provider.getTerminalView().welcome(terminalType, terminalInfos);
    }
}

/*
Run loop
*/
void ActionDispatcher::run() {
    while (true) {
        auto mode = ModeEnumMapper::toString(state.getCurrentMode());
        provider.getTerminalView().printPrompt(mode);
        std::string action = getUserAction();
        if (action.empty()) {
            continue;
        }
        dispatch(action);
    }
}

/*
Dispatch
*/
void ActionDispatcher::dispatch(const std::string& raw) {    
    if (raw.empty()) return;
    
    char first = raw[0];

    // Instructions
    if (first == '[' || first == '>' || first == '{') {
        std::vector<Instruction> instructions = provider.getInstructionTransformer().transform(raw);
        dispatchInstructions(instructions);
        return;
    }

    // Macros
    if (first == '(') {
        provider.getTerminalView().println("Macros Not Yet Implemented.");
        return;
    }

    // Terminal Command
    TerminalCommand cmd = provider.getCommandTransformer().transform(raw);
    dispatchCommand(cmd);
}

/*
Dispatch Command
*/
void ActionDispatcher::dispatchCommand(const TerminalCommand& cmd) {
    // Mode change command
    if (cmd.getRoot() == "mode" || cmd.getRoot() == "m") {
        ModeEnum maybeNewMode = provider.getUtilityController().handleModeChangeCommand(cmd);
        if (maybeNewMode != ModeEnum::None) {
            setCurrentMode(maybeNewMode);
        }
        return;
    }

    // Global command (help, P, p...)
    if (provider.getUtilityController().isGlobalCommand(cmd)) {
        provider.getUtilityController().handleCommand(cmd);
        return;
    }

    // Mode specific command
    switch (state.getCurrentMode()) {
        case ModeEnum::HIZ:
            provider.getUtilityController().handleCommand(cmd);
            break;
        case ModeEnum::OneWire:
            provider.getOneWireController().handleCommand(cmd);
            break;
        case ModeEnum::UART:
            provider.getUartController().handleCommand(cmd);
            break;
        case ModeEnum::HDUART:
            provider.getHdUartController().handleCommand(cmd);
            break;
        case ModeEnum::I2C:
            provider.getI2cController().handleCommand(cmd);
            break;
        case ModeEnum::SPI:
            provider.getSpiController().handleCommand(cmd);
            break;
        case ModeEnum::TwoWire:
            provider.getTwoWireController().handleCommand(cmd);
            break;
        case ModeEnum::ThreeWire:
            provider.getThreeWireController().handleCommand(cmd);
            break;
        case ModeEnum::DIO:
            provider.getDioController().handleCommand(cmd);
            break;
        case ModeEnum::LED:
            provider.getLedController().handleCommand(cmd);
            break;
        case ModeEnum::Infrared:
            provider.getInfraredController().handleCommand(cmd);
            break;
        case ModeEnum::USB:
            provider.getUsbController().handleCommand(cmd);
            break;
        case ModeEnum::Bluetooth:
            provider.getBluetoothController().handleCommand(cmd);
            break;
        case ModeEnum::WiFi:
            provider.getWifiController().handleCommand(cmd);
            break;
        case ModeEnum::JTAG:
            provider.getJtagController().handleCommand(cmd);
            break;
    }

   // Config was handled in specific mode, we need to rerender the pinout view
   if (cmd.getRoot() == "config" || cmd.getRoot() == "setprotocol") {
        setCurrentMode(state.getCurrentMode());
   } 
}

/*
Dispatch Instructions
*/
void ActionDispatcher::dispatchInstructions(const std::vector<Instruction>& instructions) {
    // Convert raw instructions into bytecodes vector
    auto bytecodes = provider.getInstructionTransformer().transformByteCodes(instructions);

    switch (state.getCurrentMode()) {
        case ModeEnum::OneWire:
            provider.getOneWireController().handleInstruction(bytecodes);
            break;
        case ModeEnum::UART:
            provider.getUartController().handleInstruction(bytecodes);
            break;
        case ModeEnum::HDUART:
            provider.getHdUartController().handleInstruction(bytecodes);
            break;
        case ModeEnum::I2C:
            provider.getI2cController().handleInstruction(bytecodes);
            break;
        case ModeEnum::SPI:
            provider.getSpiController().handleInstruction(bytecodes);
            break;
        case ModeEnum::TwoWire:
            provider.getTwoWireController().handleInstruction(bytecodes);
            break;
        case ModeEnum::ThreeWire:
            provider.getThreeWireController().handleInstruction(bytecodes);
            break;
        case ModeEnum::LED:
            provider.getLedController().handleInstruction(bytecodes);
            break;
        default:
            provider.getTerminalView().println("Cannot execute instruction in this mode.");
            return;
    }

    // Line by line bytecode
    provider.getTerminalView().println("");
    provider.getTerminalView().println("ByteCode Sequence:");
    for (const auto& code : bytecodes) {
        provider.getTerminalView().println(
            ByteCodeEnumMapper::toString(code.getCommand()) +
            " | data=" + std::to_string(code.getData()) +
            " | bits=" + std::to_string(code.getBits()) +
            " | repeat=" + std::to_string(code.getRepeat())
        );
    }
    provider.getTerminalView().println("");
}

/*
User Action
*/
std::string ActionDispatcher::getUserAction() {
    std::string inputLine;
    auto mode = ModeEnumMapper::toString(state.getCurrentMode());
    size_t cursorIndex = 0;

    while (true) {
        char c = provider.getTerminalInput().handler();

        if (handleEscapeSequence(c, inputLine, cursorIndex, mode)) continue;
        if (handleEnterKey(c, inputLine)) return inputLine;
        if (handleBackspace(c, inputLine, cursorIndex, mode)) continue;
        if (handlePrintableChar(c, inputLine, cursorIndex, mode));
    }
}

/*
User Action: Escape
*/
bool ActionDispatcher::handleEscapeSequence(char c, std::string& inputLine, size_t& cursorIndex, const std::string& mode) {
    if (c != '\x1B') return false;

    if (provider.getTerminalInput().readChar() == '[') {
        char next = provider.getTerminalInput().readChar();

        if (next == 'A') {
            inputLine = provider.getCommandHistoryManager().up();
            cursorIndex = inputLine.length();
        } else if (next == 'B') {
            inputLine = provider.getCommandHistoryManager().down();
            cursorIndex = inputLine.length();
        } else if (next == 'C') {
            if (cursorIndex < inputLine.length()) {
                cursorIndex++;
                provider.getTerminalView().print("\x1B[C");
            }
            return true;
        } else if (next == 'D') {
            if (cursorIndex > 0) {
                cursorIndex--;
                provider.getTerminalView().print("\x1B[D");
            }
            return true;
        } else {
            return false;
        }

        provider.getTerminalView().print("\r" + mode + "> " + inputLine + "\033[K");
        return true;
    }

    return false;
}

/*
User Action: Enter
*/
bool ActionDispatcher::handleEnterKey(char c, const std::string& inputLine) {
    if (c != '\r' && c != '\n') return false;

    provider.getTerminalView().println("");
    provider.getCommandHistoryManager().add(inputLine);
    return true;
}

/*
User Action: Backspace
*/
bool ActionDispatcher::handleBackspace(char c, std::string& inputLine, size_t& cursorIndex, const std::string& mode) {
    if (c != '\b' && c != 127) return false;
    if (cursorIndex == 0) return true;

    cursorIndex--;
    inputLine.erase(cursorIndex, 1);

    provider.getTerminalView().print("\r" + mode + "> " + inputLine + " \033[K");

    int moveBack = inputLine.length() - cursorIndex;
    for (int i = 0; i <= moveBack; ++i) {
        provider.getTerminalView().print("\x1B[D");
    }

    return true;
}

/*
User Action: Printable
*/
bool ActionDispatcher::handlePrintableChar(char c, std::string& inputLine, size_t& cursorIndex, const std::string& mode) {
    if (!isprint(c)) return false;

    inputLine.insert(cursorIndex, 1, c);
    cursorIndex++;

    provider.getTerminalView().print("\r" + mode + "> " + inputLine + "\033[K");

    int moveBack = inputLine.length() - cursorIndex;
    for (int i = 0; i < moveBack; ++i) {
        provider.getTerminalView().print("\x1B[D");
    }

    return true;
}

/*
Set Mode
*/
void ActionDispatcher::setCurrentMode(ModeEnum newMode) {
    state.setCurrentMode(newMode);

    PinoutConfig config;
    config.setMode(ModeEnumMapper::toString(newMode));
    auto proto = InfraredProtocolMapper::toString(state.getInfraredProtocol());

    switch (newMode) {
        case ModeEnum::OneWire:
            provider.getOneWireController().ensureConfigured();
            config.setMappings({ "DATA GPIO " + std::to_string(state.getOneWirePin()) });
            break;
        case ModeEnum::UART:
            provider.getUartController().ensureConfigured();
            config.setMappings({
                "TX GPIO " + std::to_string(state.getUartTxPin()),
                "RX GPIO " + std::to_string(state.getUartRxPin()),
                "BAUD " + std::to_string(state.getUartBaudRate()),
                "BITS " + std::to_string(state.getUartDataBits()),
            });
            break;
        case ModeEnum::HDUART:
            config.setMappings({
                "RX/TX GPIO " + std::to_string(state.getHdUartPin()),
                "BAUD " + std::to_string(state.getHdUartBaudRate()),
                "BITS " + std::to_string(state.getHdUartDataBits()),
                "PARITY " + state.getHdUartParity(),
            });
            provider.getHdUartController().ensureConfigured();
            break;
        case ModeEnum::I2C:
            config.setMappings({
                "SDA GPIO " + std::to_string(state.getI2cSdaPin()),
                "SCL GPIO " + std::to_string(state.getI2cSclPin()),
                "FREQ " + std::to_string(state.getI2cFrequency())
            });
            provider.getI2cController().ensureConfigured();
            break;
        case ModeEnum::SPI:
            config.setMappings({
                "MOSI GPIO " + std::to_string(state.getSpiMOSIPin()),
                "MISO GPIO " + std::to_string(state.getSpiMISOPin()),
                "SCLK GPIO " + std::to_string(state.getSpiCLKPin()),
                "CS GPIO " + std::to_string(state.getSpiCSPin())
            });
            provider.getSpiController().ensureConfigured();
            break;
        case ModeEnum::TwoWire:
            config.setMappings({"DATA GPIO", "CLK GPIO", "FREQ"}); // TODO
            provider.getTwoWireController().ensureConfigured();
            break;
        case ModeEnum::ThreeWire:
            provider.getThreeWireController().ensureConfigured();
            config.setMappings({"DATA GPIOX","CLK GPIOX","ENABLE GPIOX"}); // TODO
            break;
        case ModeEnum::LED:
            provider.getLedController().ensureConfigured();
            break;
        case ModeEnum::Infrared:
            provider.getInfraredController().ensureConfigured();
            config.setMappings(
                {"IR TX GPIO " + std::to_string(state.getInfraredTxPin()),
                 "IR RX GPIO " + std::to_string(state.getInfraredRxPin()),
                  proto
            });
            break;
        case ModeEnum::USB:
            provider.getUsbController().ensureConfigured();
            break;
        case ModeEnum::Bluetooth:
            provider.getBluetoothController().ensureConfigured();
            break;
        case ModeEnum::WiFi:
            provider.getWifiController().ensureConfigured();
            break;
        case ModeEnum::JTAG:
            config.setMappings({ "TCK GPIOX", "TMS GPIOY", "TDO GPIOZ", "TDI GPIOW" }); // TODO
            provider.getJtagController().ensureConfigured();
            break;
    }

    // Show the new mode pinout
    provider.getDeviceView().show(config);
}
