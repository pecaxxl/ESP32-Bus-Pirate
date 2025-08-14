#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ESP32Ping.h>

#include <array>
#include <string>
#include <memory>

class EthernetService {
public:
    EthernetService();

    void configure(int8_t pinCS,
                   int8_t pinRST = -1,
                   int8_t pinSCK = 18,
                   int8_t pinMISO = 19,
                   int8_t pinMOSI = 23,
                   uint32_t spiHz = 26000000UL);

    bool beginDHCP(const std::array<uint8_t,6>& mac, unsigned long timeoutMs = 10000);
    bool isConnected() const;
    void maintain();

    std::string getLocalIP()   const;
    std::string getSubnetMask()const;
    std::string getGatewayIp() const;
    std::string getDns()       const;
    std::string getMac()       const;

    int hardwareStatusRaw() const;     // Ethernet.hardwareStatus()
    int linkStatusRaw()      const;     // Ethernet.linkStatus()
    bool linkUp()            const;

private:
    void hardReset();

    // pins / conf SPI
    int8_t   _pinCS   = 5;
    int8_t   _pinRST  = -1;
    int8_t   _pinSCK  = 18;
    int8_t   _pinMISO = 19;
    int8_t   _pinMOSI = 23;
    uint32_t _spiHz   = 26000000UL;

    std::array<uint8_t,6> _mac {{0,0,0,0,0,0}};
    bool _configured = false;
};
