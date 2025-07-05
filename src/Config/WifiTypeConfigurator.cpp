#include "WifiTypeConfigurator.h"


std::string WifiTypeConfigurator::configure(TerminalTypeEnum& terminalType) {
    switch (terminalType)
    {
    case TerminalTypeEnum::WiFiClient:
        setupWifi(); // use this standalone setup for now
        return std::string(WiFi.localIP().toString().c_str());

    case TerminalTypeEnum::WiFiAp: {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("buspirate", "averylongpassword");
        return std::string(WiFi.softAPIP().toString().c_str());
    }
    default:
        break;
    }

    return "";
}
