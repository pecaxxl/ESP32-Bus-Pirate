#include "HdUartService.h"

void HdUartService::configure(unsigned long baud, uint32_t config, uint8_t pin, bool inverted) {
    ioPin = pin;
    baudRate = baud;
    serialConfig = config;
    isInverted = inverted;

    Serial1.end();
    Serial1.begin(baudRate, serialConfig, ioPin, ioPin, isInverted); // meme pin RX/TX
}

void HdUartService::switchToWrite() {
    pinMode(ioPin, OUTPUT);
    delayMicroseconds(10);
}

void HdUartService::switchToRead() {
    pinMode(ioPin, INPUT);
    delayMicroseconds(10);
}

void HdUartService::write(uint8_t data) {
    switchToWrite();
    Serial1.write(data);
    Serial1.flush();
    switchToRead();
}

void HdUartService::write(const std::string& str) {
    switchToWrite();
    Serial1.write(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
    Serial1.flush();
    switchToRead();
}

bool HdUartService::available() const {
    return Serial1.available();
}

char HdUartService::read() {
    return Serial1.read();
}

std::string HdUartService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;
    uint32_t timeout = 2000;
    uint32_t received = 0;
    uint32_t start;

    for (const ByteCode& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Write:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    write(code.getData());
                }
                break;

            case ByteCodeEnum::Read:
                start = millis();
                while (received < code.getRepeat() && (millis() - start < timeout)) {
                    if (available()) {
                        char c = read();
                        result += c;
                        received++;
                    } else {
                        delay(10);
                    }
                }
                break;

            case ByteCodeEnum::DelayMs:
                delay(code.getRepeat());
                break;

            case ByteCodeEnum::DelayUs:
                delayMicroseconds(code.getRepeat());
                break;

            default:
                break;
        }
    }

    return result;
}
