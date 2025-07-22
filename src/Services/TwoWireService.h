#pragma once

#include <Arduino.h>
#include <vector>

class TwoWireService {
public:
    typedef struct {
        uint8_t protocol_type         : 4;
        uint8_t structure_identifier  : 4;
    
        uint8_t read_with_defined_length : 1;
        uint8_t data_units_bits          : 3;
        uint8_t data_units               : 4;
    } sle44xx_atr_t;

    void configure(uint8_t clkPin, uint8_t ioPin, uint8_t rstPin);
    void end();
    
    void setRST(bool level);
    void setCLK(bool level);
    void setIO(bool level);
    bool readIO();
    
    void pulseClock();
    void sendClocks(uint16_t ticks);
    
    void writeBit(bool bit);
    bool readBit();
    void writeByte(uint8_t byte);
    uint8_t readByte();
    
    void sendStart();
    void sendStop();
    void sendCommand(uint8_t a, uint8_t b, uint8_t c);
    std::vector<uint8_t> readResponse(uint16_t len);
    
    // Smartcard
    std::vector<uint8_t> performSmartCardAtr();
    std::string parseSmartCardAtr(const std::vector<uint8_t>& atr);
    uint8_t parseSmartCardRemainingAttempts(uint8_t statusByte);
    std::string parseSmartCardStructureIdentifier(uint8_t id);
    std::vector<uint8_t> dumpSmartCardFullMemory();
    void resetSmartCard();
private:
    uint8_t clkPin;
    uint8_t ioPin;
    uint8_t rstPin;
};
