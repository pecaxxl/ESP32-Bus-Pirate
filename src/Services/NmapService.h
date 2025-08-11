#pragma once

#include <string>
#include <vector>

class NmapService {
public:
    void startTask(int verbosity);    
    bool parseHosts(const std::string& hosts_arg);
    bool parsePorts(const std::string& ports_arg);
    
private:
    // Nmap Task, cause overflow if it runs in the main loop, so it must run in a dedicated FreeRTOS task with a larger stack
    static void connectTask(void *pvParams);
    bool isIpv4(const std::string& address);
    void scanTarget(const std::string &host, const std::vector<uint16_t> &ports);

    std::vector<std::string> target_hosts;
    std::vector<uint16_t> target_ports;


    static std::vector<uint16_t> selected_ports;
};

