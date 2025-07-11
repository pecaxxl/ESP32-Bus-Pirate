#include "UartService.h"

void UartService::configure(unsigned long baud, uint32_t config, uint8_t rx, uint8_t tx, bool inverted) {
    Serial1.end(); // stop before reconfigure
    Serial1.begin(baud, config, rx, tx, inverted);
}

std::string UartService::readLine() {
    std::string input;
    bool lastWasCR = false;

    while (true) {
        if (!Serial1.available()) continue;
        
        char c = Serial1.read();

        if (c == '\r') {
            lastWasCR = true;
            Serial1.println();
            break;
        }

        if (c == '\n') {
            if (!lastWasCR) {
                Serial1.println();
                break;
            }
            continue;
        }

        if (c == '\b' || c == 127) {
            if (!input.empty()) {
                input.pop_back();
                Serial1.print("\b \b");
            }
        } else {
            input += c;
            Serial1.print(c);
            lastWasCR = false;
        }
    }

    return input;
}

void UartService::print(const std::string& msg) {
    Serial1.print(msg.c_str());
}

void UartService::println(const std::string& msg) {
    Serial1.println(msg.c_str());
}

bool UartService::available() const {
    return Serial1.available();
}

char UartService::read() {
    return Serial1.read();
}

void UartService::write(char c) {
    Serial1.write(c);
}

void UartService::write(const std::string& str) {
    Serial1.write(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

std::string UartService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;
    uint32_t timeout = 2000; // 2 secondes
    uint32_t start;
    uint32_t received = 0;

    for (const ByteCode& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Write:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    Serial1.write(code.getData());
                }
                break;

            case ByteCodeEnum::Read:
                start = millis();
                while (received < code.getRepeat() && (millis() - start < timeout)) {
                    if (Serial1.available()) {
                        char c = Serial1.read();
                        result += c;
                        ++received;
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

void UartService::switchBaudrate(unsigned long newBaud) {
    Serial1.updateBaudRate(newBaud);
}

void UartService::flush() {
    Serial.flush();
}

void UartService::clearUartBuffer() {
    const size_t maxBytes = 512; // uart buffer size ?
    size_t count = 0;
    while (count < maxBytes && available()) {
        read();
        count++;
    }
}


uint32_t UartService::buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits) {
    uint32_t config = SERIAL_8N1;
    if (dataBits == 5) config = (stopBits == 2) ? SERIAL_5N2 : SERIAL_5N1;
    else if (dataBits == 6) config = (stopBits == 2) ? SERIAL_6N2 : SERIAL_6N1;
    else if (dataBits == 7) config = (stopBits == 2) ? SERIAL_7N2 : SERIAL_7N1;
    else if (dataBits == 8) config = (stopBits == 2) ? SERIAL_8N2 : SERIAL_8N1;

    if (parity == 'E') config |= 0x02;
    else if (parity == 'O') config |= 0x01;

    return config;
}
