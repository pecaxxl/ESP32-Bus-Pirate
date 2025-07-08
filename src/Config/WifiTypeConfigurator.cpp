#include "WifiTypeConfigurator.h"


std::string WifiTypeConfigurator::configure(TerminalTypeEnum& terminalType) {
    switch (terminalType)
    {
    case TerminalTypeEnum::WiFiClient:
        #ifdef DEVICE_CARDPUTER
            // Use this standalone setup for now
            setupCardputerWifi(); // endless loop untill a WiFi is connected
        #endif

        #ifdef DEVICE_M5STICK
            // Use this standalone setup for now
            setupStickWifi(); // check stored creds
        #endif
        return std::string(WiFi.localIP().toString().c_str());

    case TerminalTypeEnum::WiFiAp: {
        return "";
    }
    default:
        break;
    }

    return "";
}
