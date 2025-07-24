#include "TerminalTypeConfigurator.h"

TerminalTypeConfigurator::TerminalTypeConfigurator(HorizontalSelector& selector)
    : selector(selector) {}

TerminalTypeEnum TerminalTypeConfigurator::configure() {
    std::vector<std::string> options = {
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::WiFiClient),
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::Serial),
    };

    int selected = 1; // Serial

    #ifdef DEVICE_M5STAMPS3
        selected = selector.selectHeadless();
    #else
        selected = selector.select(
            "ESP32 BUS PIRATE",
            options,
            "Select terminal type",
            ""
        );
    #endif

    switch (selected) {
        case 0: return TerminalTypeEnum::WiFiClient;
        case 1: return TerminalTypeEnum::Serial;
        case 2: return TerminalTypeEnum::WiFiAp;
        default: return TerminalTypeEnum::None;
    }
}
