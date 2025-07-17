#include "I2sController.h"

/*
Constructor
*/
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
    } else if (cmd.getRoot() == "test") {
        handleTest(cmd);
    } else if (cmd.getRoot() == "reset") {
        handleReset();
    } else {
        handleHelp();
    }
}


/*
Play
*/
void I2sController::handlePlay(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: play <frequency> [durationMs]");
        return;
    }

    uint16_t freq = argTransformer.parseHexOrDec32(cmd.getSubcommand());

    if (args.empty()) {
        // Lecture infinie
        terminalView.println("\nI2S Play: Tone @ " + std::to_string(freq) + " Hz (Press [ENTER] to stop)...\n");

        i2sService.playToneInterruptible(freq, 0xFFFF, [&]() -> bool {
            char ch = terminalInput.readChar();
            return ch == '\n' || ch == '\r';
        });

    } else if (args.size() == 1 && argTransformer.isValidNumber(args[0])) {
        uint16_t duration = argTransformer.parseHexOrDec32(args[0]);

        terminalView.println("\nI2S Play: Tone @ " + std::to_string(freq) + " Hz for " + std::to_string(duration) + " ms (or press [ENTER] to stop early)...\n");

        i2sService.playToneInterruptible(freq, duration, [&]() -> bool {
            char ch = terminalInput.readChar();
            return ch == '\n' || ch == '\r';
        });

    } else {
        terminalView.println("Usage: play <frequency> [durationMs]");
        return;
    }

    terminalView.println("I2S Play: Done.");
}


/*
Record
*/
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

/*
Test
*/
void I2sController::handleTest(const TerminalCommand& cmd) {
    std::string mode = cmd.getSubcommand();

    if (mode.empty()) {
        terminalView.println("Usage: test <speaker|mic>");
        return;
    }

    if (mode[0] == 's') {
        handleTestSpeaker();
    }
    else if (mode[0] == 'm') {
        handleTestMic();
    }
    else {
        terminalView.println("Usage: test <speaker|mic>");
    }
}

/*
Test Speaker
*/
void I2sController::handleTestSpeaker() {
    terminalView.println("I2S Speaker Test: Running full tests...\n");

    // Show pin config
    terminalView.println("Using pins:");
    terminalView.println("  BCLK : " + std::to_string(state.getI2sBclkPin()));
    terminalView.println("  LRCK : " + std::to_string(state.getI2sLrckPin()));
    terminalView.println("  DATA : " + std::to_string(state.getI2sDataPin()));
    terminalView.println("");

    // Melody
    terminalView.println("  Melody...");
    const uint16_t melody[] = {262, 294, 330, 349, 392, 440, 494, 523}; // C major
    for (uint16_t f : melody) {
        i2sService.playTone(f, 200);
        delay(50);
    }
    delay(400);

    // Frequency Sweep
    terminalView.println("  Frequency Sweep...");
    for (uint16_t f = 100; f <= 2000; f += 200) {
        i2sService.playTone(f, 100);
    }
    delay(400);

    // Beep Pattern
    terminalView.println("  Beep Pattern (short/long)...");
    i2sService.playTone(800, 100);
    delay(100);
    i2sService.playTone(800, 400);
    delay(200);
    i2sService.playTone(800, 100);
    delay(600);

    // Binary pattern (square wave)
    terminalView.println("  Binary tone pattern...");
    for (int i = 0; i < 5; ++i) {
        i2sService.playTone(1000, 50);
        delay(50);
    }
    delay(800);

    // Config for PCM playback
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        12000,
        16
    );

    // PCM Playback test
    terminalView.println("  PCM playback...");
    i2sService.playPcm(PcmSoundtestComplete, sizeof(PcmSoundtestComplete));

    // Restaure config
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    terminalView.println("\nI2S Speaker Test: Done.");
}

/*
Test Mic
*/
void I2sController::handleTestMic() {
    terminalView.println("\nI2S Micro: Analyzing input signal...\n");

    i2sService.configureInput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    // Show pin config
    terminalView.println("Using pins:");
    terminalView.println("  BCLK : " + std::to_string(state.getI2sBclkPin()));
    terminalView.println("  LRCK : " + std::to_string(state.getI2sLrckPin()));
    terminalView.println("  DATA : " + std::to_string(state.getI2sDataPin()));
    terminalView.println("");

    constexpr size_t sampleCount = 4096;
    std::vector<int16_t> buffer(sampleCount);
    size_t read = i2sService.recordSamples(buffer.data(), sampleCount);

    if (read == 0) {
        terminalView.println("\nI2S Micro: Failed to read from I2S microphone.");
        return;
    }

    int32_t sum = 0;
    int16_t minVal = INT16_MAX;
    int16_t maxVal = INT16_MIN;

    for (size_t i = 0; i < read; ++i) {
        int16_t val = buffer[i];
        sum += abs(val);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    int avgAmplitude = sum / read;
    int peakToPeak = maxVal - minVal;

    // Score
    std::string verdict;
    if (avgAmplitude < 30 || peakToPeak < 60) {
        verdict = "Weak or no signal detected";
    } else if (avgAmplitude > 200 && peakToPeak > 400) {
        verdict = "Strong and valid signal";
    } else {
        verdict = "Low signal, maybe too quiet?";
    }

    // Display summay
    terminalView.println("Summary:");
    terminalView.println("  Avg amplitude : " + std::to_string(avgAmplitude));
    terminalView.println("  Min value     : " + std::to_string(minVal));
    terminalView.println("  Max value     : " + std::to_string(maxVal));
    terminalView.println("  Peak-to-peak  : " + std::to_string(peakToPeak));
    terminalView.println("  Verdict       : " + verdict);

    // Reconfig output
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    terminalView.println("\nI2S Micro: Done.");
}

/*
Config
*/
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


/*
Help
*/
void I2sController::handleHelp() {
    terminalView.println("Available I2S commands:");
    terminalView.println("  play <freq> [duration]");
    terminalView.println("  record ");
    terminalView.println("  test <speaker|mic>");
    terminalView.println("  reset");
    terminalView.println("  config");
}


/*
Reset
*/
void I2sController::handleReset() {
    i2sService.end();

    // Config output
    i2sService.configureOutput(state.getI2sBclkPin(),
                         state.getI2sLrckPin(),
                         state.getI2sDataPin(),
                         state.getI2sSampleRate(),
                         state.getI2sBitsPerSample());

    terminalView.println("I2S Reset: TX (output) mode.");
}


/*
Ensure configuration
*/
void I2sController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    } else {
        // Reapply
        i2sService.end();
        i2sService.configureOutput(state.getI2sBclkPin(), state.getI2sLrckPin(),
                             state.getI2sDataPin(), state.getI2sSampleRate(),
                             state.getI2sBitsPerSample());
    }
}