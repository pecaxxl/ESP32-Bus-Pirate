#pragma once
#include <string>

enum phy_interface_t {
    phy_none,
    phy_wifi,
    phy_eth
};

enum ping_rc_t {
    ping_ok,
    ping_timeout,
    ping_resolve_fail,
    ping_session_fail,
    ping_error
};


class ICMPService {
public:
    ICMPService();
    ~ICMPService();

    // Normal ping
    void startPingTask(const std::string& host, int count = 5, int timeout_ms = 1000, int interval_ms = 200);
    // Discovery of devices
    void startDiscoveryTask(const std::string deviceIP);

    // Results
    bool isReady() const { return ready; }
    ping_rc_t lastRc() const { return pingRC; }
    int lastMedianMs() const { return pingMedianMs; }
    int lastSent() const { return pingTX; }
    int lastRecv() const { return pingRX; }
    const std::string& getReport() const { return report; }

    // Task entry
    static void pingAPI(void *pvParams);

private:
    bool ready = false;
    int  pingMedianMs = -1;
    int  pingTX = 0;
    int  pingRX = 0;
    std::string report;
    ping_rc_t pingRC = ping_rc_t::ping_error;

    void cleanupICMPService();
};
