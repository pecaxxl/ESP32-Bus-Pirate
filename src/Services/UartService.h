#pragma once
#include <string>
#include <vector>
#include "Arduino.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "hal/uart_types.h"
#include "soc/uart_periph.h"
#include "Models/ByteCode.h"

#define UART_PORT UART_NUM_1

class UartService {
public:
    void configure(unsigned long baud, uint32_t config, uint8_t rx, uint8_t tx, bool inverted);
    void print(const std::string& msg);
    void println(const std::string& msg);
    char read();
    std::string readLine();
    bool available() const;
    void write(char c);
    void write(const std::string& str);
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
    void switchBaudrate(unsigned long newBaud);
    void flush();
    void clearUartBuffer();
    void end();
    uint32_t buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits);
};
