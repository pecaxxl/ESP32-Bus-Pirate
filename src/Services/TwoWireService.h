#pragma once

#include <Arduino.h>
#include <vector>

class TwoWireService {
public:
    void configure(uint8_t clkPin, uint8_t ioPin, uint8_t rstPin);
    void end();

    void setRST(bool level);
    void setCLK(bool level);
    void setIO(bool level);
    bool readIO();

    void pulseClock();

    void writeBit(bool bit);
    bool readBit();
    void writeByte(uint8_t byte);
    uint8_t readByte();

    void sendStart();
    void sendStop();
    void sendCommand(uint8_t a, uint8_t b, uint8_t c);
    std::vector<uint8_t> readResponse(uint16_t len);
    void sendClocks(uint16_t ticks);

    std::vector<uint8_t> performATR();
    void testReadSecurityMemory();

private:
    uint8_t clkPin;
    uint8_t ioPin;
    uint8_t rstPin;
};
