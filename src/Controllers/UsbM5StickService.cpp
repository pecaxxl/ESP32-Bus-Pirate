#ifdef DEVICE_M5STICK

#include "Controllers/UsbM5StickController.h"

UsbM5StickController::UsbM5StickController(ITerminalView& terminalView)
    : terminalView(terminalView) {}

void UsbM5StickController::handleCommand(const TerminalCommand&) {
    terminalView.print("\n[INFO] USB is not supported on M5StickC Plus 2.\n\n");
}

void UsbM5StickController::ensureConfigured() {
    terminalView.print("\n[INFO] Skipping USB not supported on M5StickC Plus 2.\n\n");
}

#endif