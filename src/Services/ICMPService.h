#pragma once
#include <string>

class ICMPService {
public:
    ICMPService();

    void startPingTask(const std::string& host, int count = 5, int timeout_ms = 1000, int interval_ms = 200);

    // Results
    bool isReady() const { return ready; }
    bool lastPingUp() const { return ping_up; }
    int  lastMedianMs() const { return ping_median_ms; }
    int  lastSent() const { return ping_sent; }
    int  lastRecv() const { return ping_recv; }
    const std::string& getReport() const { return report; }

    // Task entry
    static void pingTask(void *pvParams);

private:
    bool ready = false;
    bool ping_up = false;
    int  ping_median_ms = -1;
    int  ping_sent = 0;
    int  ping_recv = 0;
    std::string report;
};
