#include "UniversalRemoteShell.h"
#include "Data/UniversalRemoteCommands.h"
#include "Data/DeviceBgoneCommands.h"

UniversalRemoteShell::UniversalRemoteShell(
    ITerminalView& view,
    IInput& input,
    InfraredService& irService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager
) : infraredService(irService),
    terminalView(view),
    terminalInput(input),
    argTransformer(argTransformer),
    userInputManager(userInputManager) {}

void UniversalRemoteShell::run() {
    // Remote actions
    const std::vector<std::string> actions = {
        "⏻ ON/OFF",
        "🔇 MUTE",
        "▶️  PLAY",
        "⏸️  PAUSE",
        "🔊 VOL UP",
        "🔉 VOL DOWN",
        "🔼 CH UP",
        "🔽 CH DOWN",
        "🚪 EXIT SHELL"
    };

    terminalView.println("INFRARED: Universal IR Remote started...\n");

    while (true) {
        // Display actions
        int index = userInputManager.readValidatedChoiceIndex("Select a remote action", actions, 0);
        if (index < 0 || index >= (int)actions.size()) {
            terminalView.println("Invalid selection.\n");
            continue;
        }

        // Handle exit
        if (actions[index] == "🚪 EXIT SHELL") {
            terminalView.println("INFRARED: Exiting infrared remote shell...\n");
            break;
        }

        // Send commands
        terminalView.println("Sending all codes for: " + actions[index] + "... Press [ENTER] to stop.\n");
        switch (index) {
            case 0: sendCommandGroup(deviceBgoneCommands); break;
            case 1: sendCommandGroup(universalMute); break;
            case 2: sendCommandGroup(universalPlay); break;
            case 3: sendCommandGroup(universalPause); break;
            case 4: sendCommandGroup(universalVolUp); break;
            case 5: sendCommandGroup(universalVolDown); break;
            case 6: sendCommandGroup(universalChannelUp); break;
            case 7: sendCommandGroup(universalChannelDown); break;
        }
    }
}

void UniversalRemoteShell::sendCommandGroup(const std::vector<InfraredCommand>& group) {
    for (const auto& cmd : group) {
        infraredService.sendInfraredCommand(cmd);
        delay(100);

        // Enter press to stop
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') {
            terminalView.println(" ⛔ Stopped by user.\n");
            return;
        }
        
        // Display sent command info
        terminalView.println(
            " ✅ Sent to protocol=" + InfraredProtocolMapper::toString(cmd.getProtocol()) +
            " device=" + std::to_string(cmd.getDevice()) +
            " sub=" + std::to_string(cmd.getSubdevice()) +
            " cmd=" + std::to_string(cmd.getFunction())
        );
    }
    terminalView.println("");
}

