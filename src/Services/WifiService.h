#pragma once

#include <WiFi.h>
#include <ESP32Ping.h>
#include <string>
#include <vector>
#include <sstream>

class WifiService {
public:
 WifiService();

    bool connect(const std::string& ssid, const std::string& password, unsigned long timeoutMs = 10000);
    void disconnect();
    bool isConnected() const;

    std::string getLocalIP() const;
    std::string getCurrentIP() const;
    std::string getMacAddress() const;

    bool startAccessPoint(const std::string& ssid, const std::string& password = "", int channel = 1, int maxConn = 4);

    std::vector<std::string> scanNetworks();
    bool ping(const std::string& host);
    void reset();
    void setModeApSta();
    void setModeApOnly();
    std::string getApIp() const;
    std::string getLocalIp() const;

private:
    bool connected;
};