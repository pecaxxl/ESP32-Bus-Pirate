#pragma once

#include <Arduino.h>
#include <vector>

class TwoWireService {
public:
    typedef struct {
        uint8_t protocol_type            : 4;
        uint8_t structure_identifier     : 4;
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
    bool waitIOHigh(uint32_t maxTicks);
    
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
    void updateSmartCardSecurityAttempts(uint8_t pattern);
    void compareSmartCardVerificationData(uint8_t address, uint8_t value);
    void writeSmartCardSecurityMemory(uint8_t address, uint8_t value);
    void writeSmartCardProtectionMemory(uint8_t address, uint8_t value);
    bool writeSmartCardMainMemory(uint8_t address, uint8_t value);
    std::vector<uint8_t> readSmartCardSecurityMemory();
    std::vector<uint8_t> readSmartCardMainMemory(uint8_t startAddress, uint16_t length);
    std::vector<uint8_t> readSmartCardProtectionMemory();
    bool protectSmartCard();
    bool unlockSmartCard(const uint8_t psc[3]);
    bool updateSmartCardPSC(const uint8_t psc[3]);
    bool getSmartCardPSC(uint8_t out_psc[3]);

private:
    uint8_t clkPin;
    uint8_t ioPin;
    uint8_t rstPin;
};
