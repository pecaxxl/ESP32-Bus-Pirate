#include "I2sController.h"

I2sController::I2sController(ITerminalView& terminalView, IInput& terminalInput,
                             I2sService& i2sService, ArgTransformer& argTransformer,
                             UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput),
      i2sService(i2sService), argTransformer(argTransformer),
      userInputManager(userInputManager) {}

void I2sController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "config") {
        handleConfig();
    } else if (cmd.getRoot() == "play") {
        handlePlay(cmd);
    } else if (cmd.getRoot() == "record") {
        handleRecord(cmd);
    } else if (cmd.getRoot() == "reset") {
        handleReset();
    } else {
        handleHelp();
    }
}

void I2sController::handleConfig() {
    terminalView.println("I2S Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t bclk = userInputManager.readValidatedPinNumber("BCLK pin", state.getI2sBclkPin(), forbidden);
    state.setI2sBclkPin(bclk);

    uint8_t lrck = userInputManager.readValidatedPinNumber("LRCK/WS pin", state.getI2sLrckPin(), forbidden);
    state.setI2sLrckPin(lrck);

    uint8_t data = userInputManager.readValidatedPinNumber("DATA pin", state.getI2sDataPin(), forbidden);
    state.setI2sDataPin(data);

    uint32_t freq = userInputManager.readValidatedUint32("Sample rate (e.g. 44100)", state.getI2sSampleRate());
    state.setI2sSampleRate(freq);

    uint8_t bits = userInputManager.readValidatedUint8("Bits per sample (e.g. 16)", state.getI2sBitsPerSample());
    state.setI2sBitsPerSample(bits);

    i2sService.configureOutput(bclk, lrck, data, freq, bits);

    terminalView.println("I2S configured.\n");
}

void I2sController::handlePlay(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (!argTransformer.isValidNumber(cmd.getSubcommand()) ||
        args.size() != 1 || !argTransformer.isValidNumber(args[0])) {
        terminalView.println("Usage: play <frequency> <durationMs>");
        return;
    }

    uint16_t freq = argTransformer.parseHexOrDec32(cmd.getSubcommand());
    uint16_t duration = argTransformer.parseHexOrDec32(args[0]);

    terminalView.println("\nI2S Play: Tone @ " + std::to_string(freq) + " Hz for " + std::to_string(duration) + " ms... Press [ENTER] to stop.\n");

    i2sService.playToneInterruptible(freq, duration, [&]() -> bool {
        char ch = terminalInput.readChar();
        return ch == '\n' || ch == '\r';
    });

    terminalView.println("I2S Play: Done.");
}

void I2sController::handleRecord(const TerminalCommand& cmd) {
    terminalView.println("I2S Record: In progress... Press [Enter] to stop.\n");

    // Config input
    i2sService.configureInput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    constexpr size_t batchSize = 2048;
    constexpr size_t groupSize = 16;
    std::vector<int16_t> buffer(batchSize);

    while (true) {
        // Read samples
        size_t samplesRead = i2sService.recordSamples(buffer.data(), batchSize);

        // Compress data
        std::string line;
        size_t samplesPerGroup = samplesRead / groupSize;

        for (size_t g = 0; g < groupSize; ++g) {
            int32_t mean = 0;
            for (size_t i = 0; i < samplesPerGroup; ++i) {
                size_t idx = g * samplesPerGroup + i;
                mean += buffer[idx];
            }
            mean /= samplesPerGroup;

            int32_t energy = 0;
            for (size_t i = 0; i < samplesPerGroup; ++i) {
                size_t idx = g * samplesPerGroup + i;
                int32_t deviation = buffer[idx] - mean;
                energy += abs(deviation);
            }
            int avg = energy / samplesPerGroup;

            int level = map(avg, 0, 32767, 0, 99);
            if (level < 0) level = 0;
            else if (level > 99) level = 99;

            if (level < 10)
                line += "[0" + std::to_string(level) + "] ";
            else
                line += "[" + std::to_string(level) + "] ";
        }

        terminalView.println(line);

        // Enter press
        char ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            break;
        }
    }

    // Reconfig ouput
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    terminalView.println("\nI2S Record: Stopped by user.\n");
}

void I2sController::handleHelp() {
    terminalView.println("Available I2S commands:");
    terminalView.println("  play <freq> [duration]");
    terminalView.println("  record ");
    terminalView.println("  reset");
    terminalView.println("  config");
}

void I2sController::handleReset() {
    i2sService.stop();

    // Config output
    i2sService.configureOutput(state.getI2sBclkPin(),
                         state.getI2sLrckPin(),
                         state.getI2sDataPin(),
                         state.getI2sSampleRate(),
                         state.getI2sBitsPerSample());

    terminalView.println("I2S Reset: TX (output) mode.");
}

void I2sController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    } else {
        // Reapply
        i2sService.stop();
        i2sService.configureOutput(state.getI2sBclkPin(), state.getI2sLrckPin(),
                             state.getI2sDataPin(), state.getI2sSampleRate(),
                             state.getI2sBitsPerSample());
    }
}