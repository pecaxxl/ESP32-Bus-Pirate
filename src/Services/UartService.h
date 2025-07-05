#pragma once
#include <string>
#include <vector>
#include "Models/ByteCode.h"
#include "Arduino.h"

class UartService {
public:
    void configure(unsigned long baud, uint32_t config, uint8_t rx, uint8_t tx, bool inverted);
    void print(const std::string& msg);
    void println(const std::string& msg);
    char read();
    std::string readLine();
    bool available() const;
    void write(char c);
    void write(const std::string& str);
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
    void switchBaudrate(unsigned long newBaud);
    void flush();
    void clearUartBuffer();
};
