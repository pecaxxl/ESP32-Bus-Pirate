#include "Controllers/EthernetController.h"

/*
Constructor
*/
EthernetController::EthernetController(ITerminalView& terminalView,
                                       IInput& terminalInput,
                                       IInput& deviceInput,
                                       EthernetService& ethernetService,
                                       ArgTransformer& argTransformer,
                                       UserInputManager& userInputManager)
: terminalView(terminalView)
, terminalInput(terminalInput)
, deviceInput(deviceInput)
, ethernetService(ethernetService)
, argTransformer(argTransformer)
, userInputManager(userInputManager)
{}

/*
Entry point for command
*/
void EthernetController::handleCommand(const TerminalCommand& cmd) {
    const auto& root = cmd.getRoot();

    if      (root == "config") handleConfig();
    else if (root == "connect")   handleConnect();
    else if (root == "status")    handleStatus();
    else if (root == "reset")     handleReset();
    else                          handleHelp();
}

/*
Connect using DHCP
*/
void EthernetController::handleConnect() {

    unsigned long timeoutMs = 5000;
    std::string macStr;
    std::array<uint8_t,6> mac = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x42 };

    auto confirmation = userInputManager.readYesNo(
        "Connect to Ethernet using DHCP with default MAC?",
        true
    );

    // Ask for MAC if not confirmed
    if (!confirmation) {
        macStr = userInputManager.readValidatedHexString(
            "MAC (AA:BB:CC:DD:EE:FF)",
            6,
            true
        );

        argTransformer.parseMac(macStr, mac);
    }

    terminalView.println("Ethernet: DHCP…");
    if (!ethernetService.beginDHCP(mac, timeoutMs)) {
        terminalView.println("Ethernet: DHCP failed.");
        return;
    }

    terminalView.println("Ethernet: Connected via DHCP.");
    terminalView.println("  IP:   " + ethernetService.getLocalIP());
    terminalView.println("  GW:   " + ethernetService.getGatewayIp());
    terminalView.println("  MASK: " + ethernetService.getSubnetMask());
    terminalView.println("  DNS:  " + ethernetService.getDns());
}

/*
Config W5500
*/
void EthernetController::handleConfig() {
    terminalView.println("\nEthernet (W5500) Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t defCS   = state.getEthernetCsPin();
    uint8_t defRST  = state.getEthernetRstPin();
    uint8_t defSCK  = state.getEthernetSckPin();
    uint8_t defMISO = state.getEthernetMisoPin();
    uint8_t defMOSI = state.getEthernetMosiPin();
    uint32_t defHz  = state.getEthernetFrequency();

    // User input for configuration
    uint8_t cs   = userInputManager.readValidatedPinNumber("W5500 CS pin",   defCS,   forbidden);
    uint8_t sck  = userInputManager.readValidatedPinNumber("W5500 SCK pin",  defSCK,  forbidden);
    uint8_t miso = userInputManager.readValidatedPinNumber("W5500 MISO pin", defMISO, forbidden);
    uint8_t mosi = userInputManager.readValidatedPinNumber("W5500 MOSI pin", defMOSI, forbidden);

    // RST optional
    bool useReset = userInputManager.readYesNo(
        "Use a RESET (RST) pin?",
        false
    );

    uint8_t rst = 255;
    if (useReset) {
        rst = userInputManager.readValidatedPinNumber("RST pin", (defRST == 255 ? 4 : defRST), forbidden);
    }

    // Frequency SPI
    uint32_t hz = userInputManager.readValidatedUint32("SPI frequency (Hz)", defHz);

    // Save
    state.setEthernetCsPin(cs);
    state.setEthernetSckPin(sck);
    state.setEthernetMisoPin(miso);
    state.setEthernetMosiPin(mosi);
    state.setEthernetRstPin(useReset ? rst : 255);
    state.setEthernetFrequency(hz);

    // Configure
    ethernetService.configure(
        cs,
        (useReset ? rst : -1),
        sck,
        miso,
        mosi,
        hz
    );

    terminalView.println("Ethernet configured.");
}

/*
W55000 Status
*/
void EthernetController::handleStatus()
{
    if (ethernetService.isConnected()) {
        terminalView.println("Ethernet: Connected");
        terminalView.println("  IP:   " + ethernetService.getLocalIP());
    } else {
        terminalView.println("Ethernet: Not connected.");
    }
    terminalView.println("  MAC:  " + ethernetService.getMac());

    auto hw = ethernetService.hardwareStatusRaw();
    auto lk = ethernetService.linkStatusRaw();
    terminalView.println("  HW:   " + std::to_string(hw) + " | LINK: " + std::to_string(lk) + (ethernetService.linkUp() ? " (UP)" : " (DOWN)"));
}

/*
Reset
*/
void EthernetController::handleReset()
{
    // ethernetService.disconnect();
    terminalView.println("Ethernet: Interface reset. Disconnected. [NYI]");
}

/*
Help
*/
void EthernetController::handleHelp() {
    terminalView.println("Ethernet commands:");
    terminalView.println("  status");
    terminalView.println("  connect");
    terminalView.println("  reset");
    terminalView.println("  config");
}

/*
Ensure W5500 is configured
*/
void EthernetController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // Reconfigure in case these pins have been used somewhere else
    auto cs = state.getEthernetCsPin();
    auto sck = state.getEthernetSckPin();
    auto miso = state.getEthernetMisoPin();
    auto mosi = state.getEthernetMosiPin();
    auto frequency = state.getEthernetFrequency();
    ethernetService.configure(cs, sck, miso, mosi, frequency);
}
