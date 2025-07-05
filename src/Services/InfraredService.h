#pragma once

#include <vector>
#include <Models/InfraredCommand.h>
#include <Vendors/MakeHex.h>
#include "Enums/InfraredProtocolEnum.h"
#include "Data/DeviceBgoneCommands.h"

class InfraredService {
public:
    void configure(uint8_t tx, uint8_t rx);
    void sendInfraredCommand(InfraredCommand command);
    InfraredCommand receiveInfraredCommand();
private:
    uint16_t getKaseikyoVendorIdCode(const std::string& input);
};



