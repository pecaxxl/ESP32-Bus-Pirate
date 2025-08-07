#pragma once

#include <string>
#include <vector>

class NmapService {
public:
    void startTask(const std::string& host, int verbosity);    
    void parseHosts(const std::string& hosts_arg);

private:
    // Nmap Task, cause overflow if it runs in the main loop, so it must run in a dedicated FreeRTOS task with a larger stack
    bool isIpv4(const std::string& address);
    bool NmapService::isIpv4(const std::string& address);

    static std::vector<uint16_t> selected_ports;
};

