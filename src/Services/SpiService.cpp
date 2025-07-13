#include "Services/SpiService.h"

void SpiService::configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency) {
    csPin = cs;
    spiFrequency = frequency;
    SPI.begin(sclk, miso, mosi, cs);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
}

void SpiService::beginTransaction() {
    SPI.beginTransaction(SPISettings(spiFrequency, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
}

void SpiService::endTransaction() {
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

uint8_t SpiService::transfer(uint8_t data) {
    return SPI.transfer(data);
}

std::string SpiService::readFlashID() {
    uint8_t id[3] = {0};

    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (uint8_t& byte : id) {
        byte = SPI.transfer(0x00);
    }
    endTransaction();

    char buf[32];
    snprintf(buf, sizeof(buf), "%02X %02X %02X", id[0], id[1], id[2]);
    return std::string(buf);
}

void SpiService::readFlashIdRaw(uint8_t* buffer) {
    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (int i = 0; i < 3; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
}

uint32_t SpiService::calculateFlashCapacity(uint8_t code) {
    // code 0x11 = 2^17 = 128 KB, etc.
    if (code >= 0x11 && code <= 0x20) {
        return 1UL << code;  // 2^code
    }
    return 0; // Non standard
}

void SpiService::readFlashData(uint32_t address, uint8_t* buffer, size_t length) {
    beginTransaction();
    SPI.transfer(0x03);  // Read Data command
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
}

void SpiService::eraseSector(uint32_t address, uint32_t freq) {
    enableWrite(freq);  // 0x06

    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x20); // Sector erase
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    waitForWriteComplete(freq);
}

void SpiService::enableWrite(uint32_t freq) {

    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x06); // Write Enable

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

void SpiService::waitForWriteComplete(uint32_t freq) {
    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x05); // Read Status Register
    while (true) {
        uint8_t status = SPI.transfer(0x00); // Dummy byte to receive status
        if ((status & 0x01) == 0) break;     // Wait until WIP bit is cleared
        delay(1);
    }

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}