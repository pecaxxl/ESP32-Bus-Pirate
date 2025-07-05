#include "WifiService.h"

WifiService::WifiService() : connected(false) {
    WiFi.mode(WIFI_STA);
}

bool WifiService::connect(const std::string& ssid, const std::string& password, unsigned long timeoutMs) {
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeoutMs) {
        delay(100);
    }

    connected = WiFi.status() == WL_CONNECTED;
    return connected;
}

void WifiService::disconnect() {
    WiFi.disconnect(true);
    connected = false;
}

bool WifiService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

std::string WifiService::getLocalIP() const {
    if (!isConnected()) return "";
    return WiFi.localIP().toString().c_str();
}

std::string WifiService::getCurrentIP() const {
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        return WiFi.softAPIP().toString().c_str();
    }
    return WiFi.localIP().toString().c_str();
}

std::string WifiService::getMacAddress() const {
    return WiFi.macAddress().c_str();
}

bool WifiService::startAccessPoint(const std::string& ssid, const std::string& password, int channel, int maxConn) {
    // WiFi.mode(WIFI_AP);
    if (password.empty()) {
        return WiFi.softAP(ssid.c_str(), nullptr, channel, false, maxConn);
    } else {
        return WiFi.softAP(ssid.c_str(), password.c_str(), channel, false, maxConn);
    }
}

bool WifiService::ping(const std::string& host) {
    IPAddress ip;
    if (!WiFi.hostByName(host.c_str(), ip)) return false;
    return Ping.ping(ip, 1);  // 1 ping
}


std::vector<std::string> WifiService::scanNetworks() {
    std::vector<std::string> results;

    int n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

    for (int i = 0; i < n; ++i) {
        std::stringstream ss;
        ss << "  SSID: " << WiFi.SSID(i).c_str()
           << " | RSSI: " << WiFi.RSSI(i)
           << " dBm | Sec: "
           << (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured");
        results.push_back(ss.str());
    }

    return results;
}

void WifiService::reset() {
    disconnect();
    WiFi.mode(WIFI_STA);
    connected = false;
}

void WifiService::setModeApSta() {
    WiFi.mode(WIFI_AP_STA);
}

void WifiService::setModeApOnly() {
    WiFi.mode(WIFI_AP);
}

std::string WifiService::getApIp() const {
    String ip = WiFi.softAPIP().toString();
    return std::string(ip.c_str());
}

std::string WifiService::getLocalIp() const {
    String ip = WiFi.localIP().toString();
    return std::string(ip.c_str());
}
