#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include "Models/ByteCode.h"

class I2cService {
public:
    void configure(uint8_t sda, uint8_t scl, uint32_t frequency = 100000);

    void beginTransmission(uint8_t address);
    void write(uint8_t data);
    bool endTransmission(bool sendStop = true);
    uint8_t requestFrom(uint8_t address, uint8_t quantity, bool sendStop = true);

    int read();
    bool available() const;
    bool end() const;

    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
};
