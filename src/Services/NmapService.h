#pragma once

#include <string>
#include <vector>

struct NmapOptions {
    bool tcp = true;         // -t sets TCP only (default)
    bool udp = false;        // -u sets UDP only
    int verbosity = 0;       // -v/-vv
    bool hasPort = false;    // Did user pass `-p` ?
    std::string ports;       // "80", "22,80-90"
    bool hasTrash = false;   // Did user pass non-option tokens?
};

class NmapService {
public:
    NmapService();
    void startTask(int verbosity);
    bool parseHosts(const std::string& hosts_arg);
    bool parsePorts(const std::string& ports_arg);
    const std::string getReport();
    const bool isReady();
    void clean();

    static NmapOptions parseNmapArgs(const std::vector<std::string>& tokens);
    void setDefaultPorts(bool tcp);
private:
    // Nmap Task, cause overflow if it runs in the main loop, so it must run in a dedicated FreeRTOS task with a larger stack
    static void scanTask(void *pvParams);
    bool isIpv4(const std::string& address);
    void scanTarget(const std::string &host, const std::vector<uint16_t> &ports);

    std::vector<std::string> target_hosts;
    std::vector<uint16_t> target_ports;
    bool ready;
    std::string report;
};

