#pragma once

#include <vector>
#include <Models/InfraredCommand.h>
#include <Vendors/MakeHex.h>
#include "Enums/InfraredProtocolEnum.h"

class InfraredService {
public:
    void configure(uint8_t tx, uint8_t rx);
    void startReceiver();
    void stopReceiver();
    void sendInfraredCommand(InfraredCommand command);
    InfraredCommand receiveInfraredCommand();
    bool receiveRaw(std::vector<uint16_t>& timings, uint32_t& khz);
    void sendRaw(const std::vector<uint16_t>& timings, uint32_t khz);
private:
    uint16_t getKaseikyoVendorIdCode(const std::string& input);
};



