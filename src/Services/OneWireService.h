#pragma once

#include <OneWire.h>
#include <vector>
#include "Models/ByteCode.h"

class OneWireService {
public:
    OneWireService();

    void configure(uint8_t pin);
    bool reset();
    void write(uint8_t data);
    void writeBytes(const uint8_t* data, uint8_t len);
    void writeByteRw1990(uint8_t pin, uint8_t data);
    void writeRw1990(uint8_t pin, uint8_t* data, size_t len);
    uint8_t read();
    void readBytes(uint8_t* buffer, uint8_t length);
    void skip();
    void select(const uint8_t rom[8]);
    uint8_t crc8(const uint8_t* data, uint8_t len);
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
    void resetSearch();
    bool search(uint8_t* rom);

private:
    OneWire* oneWire = nullptr;
    uint8_t oneWirePin = 0;
};
