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
    bool isReadableDevice(uint8_t addr, uint8_t startReg);

    void beginSlave(uint8_t address, uint8_t sda, uint8_t scl, uint32_t freq = 100000);
    void endSlave();
    void setSlaveResponse(const uint8_t* data, size_t len);
    std::vector<std::string> getSlaveLog();
    void clearSlaveLog();

    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);

private:
    static std::vector<std::string> slaveLog;
    static portMUX_TYPE slaveLogMux;
    static uint8_t slaveResponseBuffer[16];
    static size_t slaveResponseLength;
    static I2cService* activeSlaveInstance;

    static void onSlaveReceive(int len);
    static void onSlaveRequest();
};
