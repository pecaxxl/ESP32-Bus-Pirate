#ifdef DEVICE_M5STICK

#include "Controllers/UsbM5StickController.h"

UsbM5StickController::UsbM5StickController(ITerminalView& terminalView)
    : terminalView(terminalView) {}

void UsbM5StickController::handleCommand(const TerminalCommand&) {
    terminalView.println("\n  [INFO] USB is not supported on M5StickC Plus 2.");
    terminalView.println("");
}

void UsbM5StickController::ensureConfigured() {
    terminalView.println("\n  [INFO] Skipping USB not supported on M5StickC Plus 2.\n");
    terminalView.println("");
}

#endif