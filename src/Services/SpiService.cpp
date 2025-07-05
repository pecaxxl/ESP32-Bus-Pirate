#include "Services/SpiService.h"

void SpiService::configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency) {
    this->csPin = cs;
    SPI.begin(sclk, miso, mosi, cs);
    SPI.beginTransaction(SPISettings(frequency, MSBFIRST, SPI_MODE0));
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

void SpiService::readFlashID(uint8_t* buffer, size_t length) {
    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
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
