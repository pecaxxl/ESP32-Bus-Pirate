#pragma once

#include <Enums/TerminalTypeEnum.h>
#include <Vendors/CardWifiSetup.h>
#include <Vendors/StickWifiSetup.h>
#include <Vendors/S3WifiSetup.h>
#include <Vendors/TembedWifiSetup.h>
#include <string>
#include <WiFi.h>

class WifiTypeConfigurator {
public:
    WifiTypeConfigurator() = default;
    std::string configure(TerminalTypeEnum& terminalType);
};
