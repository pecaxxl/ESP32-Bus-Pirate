#include "NmapService.h"
#include <Arduino.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include <sys/ioctl.h> 
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Data/NmapUtils.h>
#include <unordered_set>

struct NmapTaskParams
{
    std::vector<std::string> target_hosts;
    std::vector<uint16_t> target_ports;
    int verbosity;
    NmapService *service;
};

void NmapService::setDefaultPorts(bool tcp){
    if (tcp) {
        this->target_ports = TOP_100_TCP_PORTS;
    } else {
        this->target_ports = TOP_100_UDP_PORTS;
    }
}

NmapOptions NmapService::parseNmapArgs(const std::vector<std::string>& tokens) {
    NmapOptions nmap_options;
    std::vector<char*> argv;

    argv.reserve(tokens.size() + 1);
    argv.push_back(const_cast<char*>("nmap"));
    for (auto& token : tokens) 
        argv.push_back(const_cast<char*>(token.c_str()));
        
    int argc = static_cast<int>(argv.size());

    // Don't print errors to stderr
    opterr = 0;
    // Restart scanning at argv[1]
    optind = 1;

    int option;
    while ((option = getopt(argc, argv.data(), "p:utv")) != -1) {
        switch (option) {
            case 'p':
                nmap_options.hasPort = true;
                nmap_options.ports = optarg ? optarg : "";
                break;
            case 's': // -sT, -sU, or -sTU
                nmap_options.tcp = false;
                nmap_options.udp = false;
                if (!optarg || !*optarg) { 
                    nmap_options.hasTrash = true; 
                    break; 
                }
                for (const char* p = optarg; *p; ++p) {
                    // Don't allow both TCP and UDP
                    if (*p == 'T' || *p == 't') {nmap_options.tcp = true; break;}
                    else if (*p == 'U' || *p == 'u') {nmap_options.udp = true; break; }
                    else nmap_options.hasTrash = true; // unknown letter after -s
                }
                break;
            case 'v':
                // If using double verbosity, it will add
                ++nmap_options.verbosity;
                break;
            default:
                // Unknown options
                nmap_options.hasTrash = true;
                break;
        }
    }

    return nmap_options;
}

NmapService::NmapService() : ready(false), arg_transformer(nullptr), verbosity(0), layer4_protocol(Layer4Protocol::TCP) {}

void NmapService::setArgTransformer(ArgTransformer& arg_transformer){
    this->arg_transformer = &arg_transformer;
}

const bool NmapService::isReady() {
    return this->ready;
}

const std::string NmapService::getReport() {
    return this->report;
}

static int udp_probe_with_timeout(in_addr addr, uint16_t port, int timeout_ms,
                                  const void* payload, size_t payload_len, int retries = 2)
{
    int s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) return nmap_rc_enum::OTHER;

    int rcvbuf = 4096;
    (void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    sockaddr_in sa{
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr   = addr 
    };

    if (::connect(s, (sockaddr*)&sa, sizeof(sa)) < 0) {
        int e = errno;
        ::close(s);
        if (e == EHOSTUNREACH || e == ENETUNREACH) 
            return nmap_rc_enum::UDP_OPEN_FILTERED;
        return nmap_rc_enum::OTHER;
    }

    timeval tv{
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000
    };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Probe message
    static const char kProbe[] = "PING\n";
    const void* buf   = (payload && payload_len) ? payload : kProbe;
    size_t      blen  = (payload && payload_len) ? payload_len : (sizeof(kProbe)-1);

    uint8_t rbuf[512];

    for (int attempt = 0; attempt <= retries; ++attempt) {
        if (::send(s, buf, blen, 0) < 0) {
            int e = errno; ::close(s);
            if (e == EHOSTUNREACH || e == ENETUNREACH)
                return nmap_rc_enum::UDP_OPEN_FILTERED;
            return nmap_rc_enum::OTHER;
        }

        ssize_t n = ::recvfrom(s, rbuf, sizeof(rbuf), MSG_TRUNC, nullptr, nullptr);
        if (n >= 0) {
            ::close(s);
            return nmap_rc_enum::UDP_OPEN;
        }

        int e = errno;
        if (e == ECONNREFUSED) {           // ICMP Unreachable
            ::close(s);
            return nmap_rc_enum::UDP_CLOSED;
        }
        if (e == EMSGSIZE) {               // Datagram larger buffer
            ::close(s);
            return nmap_rc_enum::UDP_OPEN;
        }
        if (e == EAGAIN || e == EWOULDBLOCK) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;                      // Retry
        }
        if (e == EHOSTUNREACH || e == ENETUNREACH) {
            ::close(s);
            return nmap_rc_enum::UDP_OPEN_FILTERED;
        }
        ::close(s);
        return nmap_rc_enum::OTHER;
    }

    ::close(s);
    return nmap_rc_enum::UDP_OPEN_FILTERED;
}

static int tcp_connect_with_timeout(in_addr addr, uint16_t port, int timeout_ms)
{
    int s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) 
        return nmap_rc_enum::OTHER;

    // Make socket non-blocking to avoid blocking the thread
    int nb = 1;
    if (ioctl(s, FIONBIO, &nb) < 0) {
        ::close(s);
        return nmap_rc_enum::OTHER;
    }

    // Setup the sockaddr_in structure
    sockaddr_in sa{
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr   = addr
    };

    int rc = ::connect(s, (sockaddr*)&sa, sizeof(sa));
    if (rc == 0) {
        ::close(s);
        return nmap_rc_enum::TCP_OPEN;
    }

    // Check status of connection
    if (errno != EINPROGRESS && errno != EALREADY) {
        int e = errno;
        ::close(s);
        return (e == ECONNREFUSED) ? nmap_rc_enum::TCP_CLOSED : nmap_rc_enum::OTHER;
    }

    // Wait to write
    fd_set wfds, efds;
    FD_ZERO(&wfds); FD_ZERO(&efds);
    FD_SET(s, &wfds); FD_SET(s, &efds);

    struct timeval tv{
        .tv_sec     = timeout_ms / 1000,
        .tv_usec    = (timeout_ms % 1000) * 1000
    };

    rc = ::select(s + 1, nullptr, &wfds, &efds, &tv);
    if (rc == 0) {
        ::close(s);
        return nmap_rc_enum::TCP_FILTERED;
    }
    if (rc < 0)  {
        ::close(s);
        return nmap_rc_enum::OTHER;
    }

    // Check if both stacks are set
    if (!FD_ISSET(s, &wfds) && !FD_ISSET(s, &efds)) {
        ::close(s);
        return nmap_rc_enum::OTHER;
    }

    // Check socket error
    int soerr = 0;
    socklen_t len = sizeof(soerr);
    if (getsockopt(s, SOL_SOCKET, SO_ERROR, &soerr, &len) < 0) {
        ::close(s);
        return nmap_rc_enum::OTHER;
    }

    ::close(s);
    switch (soerr)
    {
    case 0:
        /* code */
        return nmap_rc_enum::TCP_OPEN;
    case ECONNREFUSED:
    case ECONNRESET:
        return nmap_rc_enum::TCP_CLOSED;
    case ETIMEDOUT:
    case EHOSTUNREACH:
    case ENETUNREACH: 
    case EACCES:
    case EPERM:
        return nmap_rc_enum::TCP_FILTERED;
    default:
        return nmap_rc_enum::OTHER;
    }

    return nmap_rc_enum::OTHER;
}

static bool resolveIPv4(const std::string& host, in_addr& out)
{
    struct addrinfo hints{};
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* res = nullptr;
    int rc = getaddrinfo(host.c_str(), nullptr, &hints, &res);
    if (rc != 0 || !res) return false;

    out = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
    return true;
}

static inline void trimWhitespaces(std::string& input_str) {
    if (input_str.empty()) 
        return;

    const char* whitespaceCharacters = " \t\r\n";
    input_str.erase(0, input_str.find_first_not_of(whitespaceCharacters));

    size_t pos = input_str.find_last_not_of(whitespaceCharacters);
    if (pos != std::string::npos) 
        input_str.erase(pos + 1); 
    else 
        input_str.clear();
}

void NmapService::scanTarget(const std::string &host, const std::vector<uint16_t> &ports)
{
    in_addr ip{};
    if (!resolveIPv4(host, ip)) {
        this->report.append("Failed to resolve host: ").append(host).append("\r\n");
        return;
    }

    char ipStr[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &ip, ipStr, sizeof(ipStr));
    this->report.append("Scanning host: ").append(host).append(" (").append(ipStr).append(")\r\n");

    int closed_ports = ports.size();
    for (uint16_t p : ports) {
        //this->report.append("Scanning port ").append(std::to_string(p)).append("...\r\n");

        if (this->layer4_protocol == Layer4Protocol::TCP) {
            int st = tcp_connect_with_timeout(ip, p, CONNECT_TIMEOUT_MS);
            switch (st) {
                case nmap_rc_enum::TCP_OPEN:  
                    this->report.append("Port ").append(std::to_string(p)).append("/tcp OPEN\r\n"); 
                    closed_ports--;
                    break;
                case nmap_rc_enum::TCP_CLOSED:
                    if (this->verbosity >= 1)
                        this->report.append("Port ").append(std::to_string(p)).append("/tcp CLOSED\r\n");
                    break;
                case nmap_rc_enum::TCP_FILTERED:
                    this->report.append("Port ").append(std::to_string(p)).append("/tcp FILTERED\r\n");
                    closed_ports--;
                    break;
                default: 
                    if (this->verbosity >= 1)
                        this->report.append("Port ").append(std::to_string(p)).append("/tcp ERROR\r\n"); break;
            }
        }
        else if (this->layer4_protocol == Layer4Protocol::UDP) {
            int st = udp_probe_with_timeout(ip, p, CONNECT_TIMEOUT_MS, nullptr, 0);

            // TODO add custom payload per protocol
            switch (st) {
                case nmap_rc_enum::UDP_OPEN:
                    this->report.append("Port ").append(std::to_string(p)).append("/udp OPEN\r\n"); 
                    closed_ports--;
                    break;
                case nmap_rc_enum::UDP_CLOSED:
                    if (this->verbosity >= 1)
                        this->report.append("Port ").append(std::to_string(p)).append("/udp CLOSED\r\n");
                    break;
                case nmap_rc_enum::UDP_OPEN_FILTERED:
                    this->report.append("Port ").append(std::to_string(p)).append("/udp OPEN|FILTERED\r\n");
                    closed_ports--;
                    break;
                default:
                    if (this->verbosity >= 1)
                        this->report.append("Port ").append(std::to_string(p)).append("/udp ERROR\r\n");
                    break;
            }
        }
        else {
            // Not an implemented layer 4 protocol
            return;
        }
        // Yield
        vTaskDelay(pdMS_TO_TICKS(SMALL_DELAY_MS));
    }
    
    if (closed_ports > 0)
        this->report.append("Closed ports: ").append(std::to_string(closed_ports)).append("\r\n\n");

}

void NmapService::clean()
{
    this->target_hosts.clear();
    this->target_ports.clear();
    this->report.clear();
    this->ready = false;
    this->layer4_protocol = Layer4Protocol::TCP;
    this->verbosity = 0;
}

void NmapService::scanTask(void *pvParams)
{
    auto *params = static_cast<NmapTaskParams *>(pvParams);
    auto &service = *params->service;
    auto &hosts = params->target_hosts;
    for (auto host : hosts) {
        service.scanTarget(host, params->target_ports);
    }

    service.ready = true;
    service.verbosity = params->verbosity;
    delete params;
    vTaskDelete(nullptr);
}

void NmapService::startTask(int verbosity)
{
    auto *params = new NmapTaskParams{this->target_hosts, this->target_ports, verbosity, this};
    xTaskCreatePinnedToCore(scanTask, "NmapConnect", 20000, params, 1, nullptr, 1);
    delay(100); // start task delay
}

bool NmapService::parseHosts(const std::string& hosts_arg)
{
    this->target_hosts = std::vector<std::string>();

    // If we find ',' or '-' or '/network_mask' there are multiple hosts
    if (hosts_arg.find(',') == std::string::npos && hosts_arg.find('-') == std::string::npos && hosts_arg.find('/') == std::string::npos) {
        // Single host
        if (isIpv4(hosts_arg)) {
            this->target_hosts.push_back(hosts_arg);
        }
        else {
            return false;
        }
    }
    else {
        // Not yet implemented
        return false;
    }
    return true;
}

void NmapService::setLayer4(bool layer4_protocol){
    if (layer4_protocol == true){
        this->layer4_protocol = Layer4Protocol::TCP;
    }
    else {
        this->layer4_protocol = Layer4Protocol::UDP;
    }
}

bool NmapService::parsePorts(const std::string& ports_arg)
{
    this->target_ports = std::vector<uint16_t>();

    if (ports_arg.empty())
        return false;

    std::unordered_set<uint16_t> seen;
    std::stringstream portsStream(ports_arg);
    std::string token;

    while (std::getline(portsStream, token, ',')) {
        trimWhitespaces(token);
        if (token.empty()) continue;

        // Range?
        size_t dash = token.find('-');
        if (dash != std::string::npos) {
            std::string a = token.substr(0, dash);
            std::string b = token.substr(dash + 1);
            trimWhitespaces(a);
            trimWhitespaces(b);

            uint16_t port1 = this->arg_transformer->parseHexOrDec16(a);
            uint16_t port2 = this->arg_transformer->parseHexOrDec16(b);

            if (!port1 || !port2)
                return false;

            if (port1 > port2)
                std::swap(port1, port2);

            for (uint16_t port = port1; port <= port2; ++port) {
                if (seen.insert(port).second)
                    target_ports.push_back(port);
            }
        } else {
            // Single port
            uint16_t port = this->arg_transformer->parseHexOrDec16(token);
            if (!port)
                return false;
            if (seen.insert(port).second)
                target_ports.push_back(port);
        }
    }

    if (target_ports.empty()) 
        return false;

    std::sort(target_ports.begin(), target_ports.end());
    return true;
}


bool NmapService::isIpv4(const std::string& address)
{
    // Check if the address is a valid IPv4 address
    struct sockaddr_in sa;
    return inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0;
}