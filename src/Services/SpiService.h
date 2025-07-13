#pragma once

#include <Arduino.h>
#include <SPI.h>

class SpiService {
public:
    void configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency = 1000000);

    void beginTransaction();
    void endTransaction();

    uint8_t transfer(uint8_t data);

    std::string readFlashID();
    void readFlashIdRaw(uint8_t* buffer);
    void readFlashData(uint32_t address, uint8_t* buffer, size_t length);
    uint32_t calculateFlashCapacity(uint8_t code);

private:
    uint8_t csPin;
    uint32_t spiFrequency = 1000000;
};
