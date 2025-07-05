// OneWireService.cpp

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

void OneWireService::writeByteRw1990(uint8_t pin, uint8_t data) {
    int data_bit;
    for (data_bit = 0; data_bit < 8; data_bit++) {
        if (data & 1) {
            digitalWrite(pin, LOW);
            pinMode(pin, OUTPUT);
            delayMicroseconds(60);
            pinMode(pin, INPUT);
            digitalWrite(pin, HIGH);
        } else {
            digitalWrite(pin, LOW);
            pinMode(pin, OUTPUT);
            pinMode(pin, INPUT);
            digitalWrite(pin, HIGH);
        }
        delay(10);
        data = data >> 1;
    }
}

void OneWireService::writeRw1990(uint8_t pin, uint8_t* data, size_t len) {
    if (!reset()) return;

    // Prepare
    oneWire->skip();
    oneWire->reset();
    oneWire->write(0x33); // Read ROM

    // Write mode
    oneWire->skip();
    oneWire->reset();
    oneWire->write(0x3C); // Set write mode for some models
    delay(50);

    // Write Command ?
    oneWire->skip();
    oneWire->reset();
    oneWire->write(0xD1); // Write command
    delay(50);

    // Write don't work without this code
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    delayMicroseconds(60);
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
    delay(10);

    oneWire->skip();
    oneWire->reset();
    oneWire->write(0xD5); // Enter write mode
    delay(50);

    // Write
    for (size_t i = 0; i < len; ++i) {
        writeByteRw1990(pin, data[i]);
        delayMicroseconds(25);
    }

    oneWire->reset(); // Reset bus
    oneWire->skip();

    // Finalise
    oneWire->write(0xD1); // End of write command
    delayMicroseconds(16);
    oneWire->reset(); // Reset bus
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

