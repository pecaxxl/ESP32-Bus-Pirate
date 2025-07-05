#include "I2cService.h"

void I2cService::configure(uint8_t sda, uint8_t scl, uint32_t frequency) {
    Wire.end();
    Wire.begin(sda, scl, frequency);
}

void I2cService::beginTransmission(uint8_t address) {
    Wire.beginTransmission(address);
}

void I2cService::write(uint8_t data) {
    Wire.write(data);
}

bool I2cService::endTransmission(bool sendStop) {
    return Wire.endTransmission(sendStop);
}

uint8_t I2cService::requestFrom(uint8_t address, uint8_t quantity, bool sendStop) {
    return Wire.requestFrom(address, quantity, sendStop);
}

int I2cService::read() {
    return Wire.read();
}

bool I2cService::available() const {
    return Wire.available();
}

bool I2cService::end() const {
    return Wire.end();
}

std::string I2cService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;
    uint8_t currentAddress = 0;
    bool transmissionStarted = false;
    bool expectAddress = false;

    for (const auto& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Start:
                expectAddress = true;
                break;

            case ByteCodeEnum::Stop:
                if (transmissionStarted) {
                    Wire.endTransmission();
                    transmissionStarted = false;
                }
                break;

            case ByteCodeEnum::Write:
                if (expectAddress) {
                    currentAddress = code.getData();
                    Wire.beginTransmission(currentAddress);
                    transmissionStarted = true;
                    expectAddress = false;
                } else {
                    if (!transmissionStarted) {
                        Wire.beginTransmission(currentAddress);
                        transmissionStarted = true;
                    }
                    for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                        Wire.write(code.getData());
                    }
                }
                break;

                case ByteCodeEnum::Read: {
                    if (transmissionStarted) {
                        Wire.endTransmission(false);  // release bus
                        transmissionStarted = false;
                    }

                    uint8_t toRead = code.getRepeat();
                    uint8_t readAddr = currentAddress;

                    Wire.requestFrom(readAddr, toRead);
                    
                    for (uint8_t i = 0; i < toRead && Wire.available(); ++i) {
                        uint8_t val = Wire.read();
                        char hex[5];
                        snprintf(hex, sizeof(hex), "%02X ", val);
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

    // If no end stop
    if (transmissionStarted) {
        Wire.endTransmission();
    }

    return result;
}

