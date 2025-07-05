#include "TerminalTypeConfigurator.h"

TerminalTypeConfigurator::TerminalTypeConfigurator(HorizontalSelector& selector)
    : selector(selector) {}

TerminalTypeEnum TerminalTypeConfigurator::configure() {
    std::vector<std::string> options = {
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::WiFiClient),
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::Serial),
    };

    int selected = selector.select(
        "ESP32 Bus Pirate",
        options,
        "Select terminal type",
        ""
    );

    switch (selected) {
        case 0: return TerminalTypeEnum::WiFiClient;
        case 1: return TerminalTypeEnum::Serial;
        case 2: return TerminalTypeEnum::WiFiAp;
        default: return TerminalTypeEnum::None;
    }
}
