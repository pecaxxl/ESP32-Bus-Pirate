#pragma once

#include <vector>
#include <Arduino.h>
#include <SPI.h>
#include <Data/FlashDatabase.h>
#include <Models/ByteCode.h>
#include <atomic>
#include <M5Unified.h>
#include <deque>
#include <mutex>

class SpiService {
public:
    void configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency = 1000000);
    void end();

    void beginTransaction();
    void endTransaction();

    // Flash
    uint8_t transfer(uint8_t data);
    std::string readFlashID();
    void readFlashIdRaw(uint8_t* buffer);
    void readFlashData(uint32_t address, uint8_t* buffer, size_t length);
    uint32_t calculateFlashCapacity(uint8_t code);
    void eraseSector(uint32_t address, uint32_t freq);
    void enableWrite(uint32_t freq);
    void waitForWriteComplete(uint32_t freq);

    // Write page
    void writeFlashPage(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq);

    // Write patch, we save a sector data, modify data, erase sector, write modified data to sector
    void writeFlashPatch(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq);

    // Slave
    void startSlave(int sclk, int miso, int mosi, int cs);
    void stopSlave(int sclk, int miso, int mosi, int cs);
    bool isSlave() const;
    std::vector<std::vector<uint8_t>> getSlaveData();

    // Instructions
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
private:
    uint8_t csPin;
    uint32_t spiFrequency = 1000000;
    bool slaveConfigured = false;
};
