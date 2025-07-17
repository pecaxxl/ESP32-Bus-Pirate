#include "TwoWireService.h"
#include <Arduino.h>

void TwoWireService::configure(uint8_t clk, uint8_t io, uint8_t rst) {
    clkPin = clk;
    ioPin = io;
    rstPin = rst;

    gpio_reset_pin((gpio_num_t)clkPin);
    gpio_reset_pin((gpio_num_t)rstPin);
    gpio_reset_pin((gpio_num_t)ioPin);

    gpio_set_direction((gpio_num_t)clkPin, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)rstPin, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)ioPin, GPIO_MODE_OUTPUT);

    gpio_set_level((gpio_num_t)clkPin, 0);
    gpio_set_level((gpio_num_t)rstPin, 0);
    gpio_set_level((gpio_num_t)ioPin, 1); // release
}

void TwoWireService::end() {
    gpio_set_level((gpio_num_t)clkPin, 0);
    gpio_set_level((gpio_num_t)rstPin, 0);

    gpio_set_direction((gpio_num_t)clkPin, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)clkPin, GPIO_FLOATING);

    gpio_set_direction((gpio_num_t)rstPin, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)rstPin, GPIO_FLOATING);

    gpio_set_direction((gpio_num_t)ioPin, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)ioPin, GPIO_FLOATING);
}

void TwoWireService::setRST(bool level) {
    gpio_set_level((gpio_num_t)rstPin, level ? 1 : 0);
}

void TwoWireService::setCLK(bool level) {
    gpio_set_level((gpio_num_t)clkPin, level ? 1 : 0);
}

void TwoWireService::setIO(bool level) {
    if (level) {
        gpio_set_direction((gpio_num_t)ioPin, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)ioPin, GPIO_PULLUP_ONLY);
    } else {
        gpio_set_pull_mode((gpio_num_t)ioPin, GPIO_FLOATING);
        gpio_set_direction((gpio_num_t)ioPin, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)ioPin, 0);
    }
}

bool TwoWireService::readIO() {
    gpio_set_direction((gpio_num_t)ioPin, GPIO_MODE_INPUT);
    return gpio_get_level((gpio_num_t)ioPin);
}

void TwoWireService::pulseClock() {
    setCLK(false);
    delayMicroseconds(5);
    setCLK(true);
    delayMicroseconds(5);
    setCLK(false);
}

void TwoWireService::writeBit(bool bit) {
    setIO(bit);
    pulseClock();
}

bool TwoWireService::readBit() {
    setIO(true);
    delayMicroseconds(5);
    setCLK(true);
    delayMicroseconds(5);
    bool bit = readIO();
    setCLK(false);
    delayMicroseconds(5);
    return bit;
}

void TwoWireService::writeByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        writeBit((byte >> i) & 1);
    }
}

uint8_t TwoWireService::readByte() {
    uint8_t b = 0;
    for (uint8_t i = 0; i < 8; i++) {
        bool bit = readBit();
        if (bit) b |= (1 << i);
    }
    return b;
}

void TwoWireService::sendStart() {
    setIO(true);
    setCLK(true);
    delayMicroseconds(5);
    setIO(false);
    delayMicroseconds(5);
    setCLK(false);
}

void TwoWireService::sendStop() {
    setIO(false);
    setCLK(true);
    delayMicroseconds(5);
    setIO(true);
    delayMicroseconds(5);
    setCLK(false);
}

void TwoWireService::sendCommand(uint8_t a, uint8_t b, uint8_t c) {
    Serial.printf("Sending command: 0x%02X 0x%02X 0x%02X\n\r", a, b, c);
    sendStart();
    writeByte(a);
    writeByte(b);
    writeByte(c);
    sendStop();
}

std::vector<uint8_t> TwoWireService::readResponse(uint16_t len) {
    std::vector<uint8_t> data;
    for (uint16_t i = 0; i < len; i++) {
        data.push_back(readByte());
    }
    return data;
}

void TwoWireService::sendClocks(uint16_t ticks) {
    for (uint16_t i = 0; i < ticks; i++) {
        pulseClock();
    }
}

std::vector<uint8_t> TwoWireService::performATR() {
    std::vector<uint8_t> atr;

    setRST(false);    // RST LOW
    delay(1);         // wait

    setRST(true);     // RST release
    pulseClock();     // tick clock

    delayMicroseconds(50);
    setRST(false);    // RST LOW

    // Read 4 bytes lsb first
    return readResponse(4);
}

void TwoWireService::testReadSecurityMemory() {
    Serial.println("==> Reading SLE4442 security memory (0x31 0x00 0x00)");

    sendCommand(0x31, 0x00, 0x00);
    std::vector<uint8_t> sec = readResponse(4);
    Serial.print("Security: ");
    for (uint8_t b : sec) Serial.printf("0x%02X ", b);
    Serial.println();

    if (sec.size() >= 1) {
        uint8_t attempts = sec[0];
        Serial.printf("Remaining unlock attempts: %d (0x%02X)\n", attempts & 0x07, attempts);
    }
}
