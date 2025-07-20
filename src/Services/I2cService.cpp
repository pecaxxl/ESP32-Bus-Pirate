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

bool I2cService::isReadableDevice(uint8_t addr, uint8_t startReg) {
    beginTransmission(addr);
    write(startReg);
    bool writeOk = (endTransmission(false) == 0);
    if (!writeOk) return false;

    uint8_t received = requestFrom(addr, 1, true);
    if (received != 1 || !available()) return false;

    read();  // read to flush
    return true;
}


/*
Slave
*/
std::vector<std::string> I2cService::slaveLog;
uint8_t I2cService::slaveResponseBuffer[16] = {};
size_t I2cService::slaveResponseLength = 1;
portMUX_TYPE I2cService::slaveLogMux = portMUX_INITIALIZER_UNLOCKED;

void I2cService::beginSlave(uint8_t address, uint8_t sda, uint8_t scl, uint32_t freq) {
    Wire.end();
    Wire1.end();
    Wire1.begin(address, sda, scl, freq);

    Wire1.onReceive(onSlaveReceive);
    Wire1.onRequest(onSlaveRequest);
}

void I2cService::endSlave() {
    Wire1.end();
}

void I2cService::setSlaveResponse(const uint8_t* data, size_t len) {
    size_t copyLen = (len < sizeof(slaveResponseBuffer)) ? len : sizeof(slaveResponseBuffer);
    memcpy(slaveResponseBuffer, data, copyLen);
    slaveResponseLength = copyLen;
}

std::vector<std::string> I2cService::getSlaveLog() {
    portENTER_CRITICAL(&slaveLogMux);
    std::vector<std::string> copy = slaveLog;
    portEXIT_CRITICAL(&slaveLogMux);
    return copy;
}

void I2cService::clearSlaveLog() {
    portENTER_CRITICAL(&slaveLogMux);
    slaveLog.clear();
    portEXIT_CRITICAL(&slaveLogMux);
}

void I2cService::onSlaveReceive(int len) {
    std::string entry = "Master wrote:";
    while (Wire1.available()) {
        uint8_t b = Wire1.read();
        char hex[5];
        snprintf(hex, sizeof(hex), " %02X", b);
        entry += hex;
    }

    portENTER_CRITICAL(&slaveLogMux);
    slaveLog.push_back(entry);
    portEXIT_CRITICAL(&slaveLogMux);
}

void I2cService::onSlaveRequest() {
    portENTER_CRITICAL(&slaveLogMux);
    slaveLog.push_back("Master requested read");
    portEXIT_CRITICAL(&slaveLogMux);

    Wire1.write(slaveResponseBuffer, slaveResponseLength);
}