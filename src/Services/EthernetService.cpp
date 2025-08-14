#include "EthernetService.h"

EthernetService::EthernetService() {}

void EthernetService::configure(int8_t pinCS, int8_t pinRST, int8_t pinSCK, int8_t pinMISO, int8_t pinMOSI, uint32_t spiHz) {
    _pinCS  = pinCS;
    _pinRST = pinRST;
    _pinSCK = pinSCK;
    _pinMISO= pinMISO;
    _pinMOSI= pinMOSI;
    _spiHz  = spiHz;

    SPI.end();
    SPI.begin(_pinSCK, _pinMISO, _pinMOSI, _pinCS);

    Ethernet.init(_pinCS);

    hardReset();

    _configured = true;
}

void EthernetService::hardReset() {
    if (_pinRST < 0) return;
    pinMode(_pinRST, OUTPUT);
    digitalWrite(_pinRST, LOW);
    delay(5);
    digitalWrite(_pinRST, HIGH);
    delay(200);
}

bool EthernetService::beginDHCP(const std::array<uint8_t,6>& mac, unsigned long timeoutMs) {
    if (!_configured) return false;
    _mac = mac;

    int ret = Ethernet.begin(const_cast<uint8_t*>(_mac.data()),
                         timeoutMs,
                         4000);          // responseTimeout
                         
    if (ret != 1) return false;
    return true;
}

bool EthernetService::isConnected() const {
    return  linkUp() && Ethernet.localIP();
}

void EthernetService::maintain() {
    Ethernet.maintain();
}

int EthernetService::hardwareStatusRaw() const {
    return (int)Ethernet.hardwareStatus();
}
int EthernetService::linkStatusRaw() const {
    return (int)Ethernet.linkStatus();
}
bool EthernetService::linkUp() const {
    return Ethernet.linkStatus() == LinkON;
}

std::string EthernetService::getLocalIP() const {
    return Ethernet.localIP().toString().c_str();
}
std::string EthernetService::getSubnetMask() const {
    return Ethernet.subnetMask().toString().c_str();
}
std::string EthernetService::getGatewayIp() const {
    return Ethernet.gatewayIP().toString().c_str();
}
std::string EthernetService::getDns() const {
    return Ethernet.dnsServerIP().toString().c_str();
}
std::string EthernetService::getMac() const {
    char buf[18];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
            _mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5]);
    return std::string(buf);
}