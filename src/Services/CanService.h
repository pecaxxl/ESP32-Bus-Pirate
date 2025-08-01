#pragma once
#include <string>
#include <vector>
#include <mcp2515.h>
#include <Arduino.h>

// Must be global to work, cs pin needs to be set at compile time
MCP2515 mcp2515 = MCP2515(CAN_CS_PIN); 

class CanService {
public:
    void configure(uint8_t csPin, uint8_t sck, uint8_t miso, uint8_t mosi, uint32_t bitrateKbps = 125);
    void end();
    bool sendFrame(uint32_t id, const std::vector<uint8_t>& data);
    bool readFrame(struct can_frame& outFrame);
    std::string readFrameAsString();  // pour affichage
    void setBitrate(uint32_t bitrateKbps);
    void reset();
    void flush();
    void setFilter(uint32_t id);
    void setMask(uint32_t mask);
    std::string getStatus();
    uint32_t closestSupportedBitrate(uint32_t kbps);
private:
    CAN_SPEED resolveBitrate(uint32_t kbps);
    uint8_t csPin, sckPin, misoPin, mosiPin;
    uint32_t kbps;

};
