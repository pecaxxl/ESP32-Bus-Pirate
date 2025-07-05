#pragma once
#include <string>

enum class TerminalTypeEnum {
    Serial,
    WiFiAp,
    WiFiClient,
    None
};

class TerminalTypeEnumMapper {
public:
    static std::string toString(TerminalTypeEnum type) {
        switch (type) {
            case TerminalTypeEnum::Serial:      return "USB Serial";
            case TerminalTypeEnum::WiFiAp:      return "WiFi AP";
            case TerminalTypeEnum::WiFiClient:  return "WiFi Web";
            case TerminalTypeEnum::None:        return "None";
            default:                            return "Unknown";
        }
    }
};
