#pragma once

#include <Enums/TerminalTypeEnum.h>
#include <Vendors/CardWifiSetup.h>
#include <string>
#include <WiFi.h>

class WifiTypeConfigurator {
public:
    WifiTypeConfigurator() = default;
    std::string configure(TerminalTypeEnum& terminalType);
};
