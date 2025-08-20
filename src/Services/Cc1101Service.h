#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "ELECHOUSE_CC1101_SRC_DRV.h"

class CC1101Service {
public:
    // Base
    void configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint8_t gdo0, uint8_t gdo2, uint32_t frequency = 1000000);
    void send(const std::string& msg);

private:
    uint8_t csPin;
    uint32_t spiFrequency = 1000000;

};