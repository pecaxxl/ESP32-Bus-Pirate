#include "WifiTypeConfigurator.h"


std::string WifiTypeConfigurator::configure(TerminalTypeEnum& terminalType) {
    switch (terminalType)
    {
    case TerminalTypeEnum::WiFiClient:
        setupWifi();
        return std::string(WiFi.localIP().toString().c_str());

    case TerminalTypeEnum::WiFiAp: {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("buspirate", "averylongpass");
        return std::string(WiFi.softAPIP().toString().c_str());
    }
    default:
        break;
    }

    return "";
}
