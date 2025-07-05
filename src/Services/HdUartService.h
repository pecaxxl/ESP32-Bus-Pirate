#pragma once

#include <vector>
#include "Models/ByteCode.h"
#include <Arduino.h>
class HdUartService {
public:
    void configure(unsigned long baud, uint32_t config, uint8_t pin, bool inverted);
    void write(uint8_t data);
    void write(const std::string& str);
    bool available() const;
    char read();
    std::string readLine();
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
    void flush();

private:
    uint8_t ioPin;
    unsigned long baudRate;
    uint32_t serialConfig;
    bool isInverted;

    void switchToRead();
    void switchToWrite();
};
