#pragma once

#include <string>
#include <vector>
#include "Transformers/ArgTransformer.h"
#include "Services/ICMPService.h"

struct NmapOptions {
    bool tcp = true;        // -t sets TCP only (default)
    bool udp = false;       // -u sets UDP only
    int verbosity = 0;      // -v/-vv
    bool hasPort = false;   // Did user pass `-p` ?
    std::string ports;      // "80", "22,80-90"
    bool hasTrash = false;  // Did user pass non-option tokens?
    bool help = false;      // -h or --help
    bool pingOnly = false;  // -sn
};

enum class Layer4Protocol {
    TCP,
    UDP
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
    void setArgTransformer(ArgTransformer& argTransformer);
    void setICMPService(ICMPService* icmpService);
    void setLayer4(bool layer4Protocol);
    std::string getHelpText();

private:
    // Nmap Task, cause overflow if it runs in the main loop, so it must run in a dedicated FreeRTOS task with a larger stack
    static void scanTask(void *pvParams);
    bool isIpv4(const std::string& address);
    void scanTarget(const std::string &host, const std::vector<uint16_t> &ports);

    ICMPService* icmpService;
    std::vector<std::string> targetHosts;
    std::vector<uint16_t> targetPorts;
    bool ready;
    std::string report;
    Layer4Protocol layer4Protocol;
    ArgTransformer* argTransformer;
    int verbosity;
};

