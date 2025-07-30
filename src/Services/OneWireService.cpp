#include "OneWireService.h"

OneWireService::OneWireService() {}

void OneWireService::configure(uint8_t pin) {
    if (oneWire != nullptr) {
        delete oneWire;
        oneWire = nullptr;
    }
    oneWirePin = pin;
    oneWire = new OneWire(pin);
}

bool OneWireService::reset() {
    if (oneWire) return oneWire->reset();
}

void OneWireService::write(uint8_t data) {
    if (oneWire) oneWire->write(data);
}

void OneWireService::writeBytes(const uint8_t* data, uint8_t len) {
    if (oneWire) oneWire->write_bytes(data, len);
}

void OneWireService::writeRw1990(uint8_t pin, uint8_t* data, size_t len) {
    // Based on: https://github.com/ArminJo/iButtonProgrammer/blob/master/iButtonProgrammer.ino
    // @ArminJo

    // Configure for bit bang
    pinMode(pin, INPUT_PULLUP);
    delay(10);

    oneWire->reset();
    delay(1);

    // Read is necessary before write
    oneWire->write(0x33);
    for (uint8_t i = 0; i < 8; i++) {
        oneWire->read();
    }
    delay(5);

    // Reset for write
    oneWire->reset();
    delay(1);

    // Write mode
    oneWire->write(0xD5);

    // Bit-bang each byte
    for (size_t byteIndex = 0; byteIndex < len; byteIndex++) {
        uint8_t byte = data[byteIndex];
        for (uint8_t bit = 0; bit < 8; bit++) {
            bool bitValue = byte & 0x01;

            if (bitValue) {
                pinMode(pin, OUTPUT);
                digitalWrite(pin, LOW);
                delayMicroseconds(60);
                pinMode(pin, INPUT);
                delay(10);
            } else {
                pinMode(pin, OUTPUT);
                digitalWrite(pin, LOW);
                pinMode(pin, INPUT);
                delay(10);
            }

            byte >>= 1;
        }
    }

    delay(5);
    oneWire->reset();
}

uint8_t OneWireService::read() {
    return oneWire ? oneWire->read() : 0;
}

void OneWireService::readBytes(uint8_t* buffer, uint8_t length) {
    oneWire->read_bytes(buffer, length);
}

void OneWireService::skip() {
    if (oneWire) oneWire->skip();
}

void OneWireService::select(const uint8_t rom[8]) {
    if (oneWire) oneWire->select(rom);
}

uint8_t OneWireService::crc8(const uint8_t* data, uint8_t len) {
    if (!oneWire) return 0;
    return OneWire::crc8(data, len);
}

std::string OneWireService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;

    for (const auto& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Start:
            case ByteCodeEnum::Stop:
                reset();
                break;

            case ByteCodeEnum::Write: {
                uint8_t byte = static_cast<uint8_t>(code.getData());
                oneWire->write(byte);
                break;
            }

            case ByteCodeEnum::Read: {
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    uint8_t value = read();
                    char hex[4];
                    snprintf(hex, sizeof(hex), "%02X ", value);
                    result += hex;
                }
                break;
            }

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

void OneWireService::resetSearch() {
    if (oneWire) oneWire->reset_search();
}

bool OneWireService::search(uint8_t* rom) {
    if (!oneWire) return false;
    return oneWire->search(rom);
}

