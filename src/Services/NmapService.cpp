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

struct NmapTaskParams
{
    std::vector<std::string> target_hosts;
    std::vector<uint16_t> target_ports;
    int verbosity;
    NmapService *service;
};

NmapService::NmapService() : ready(false) {}

const bool NmapService::isReady() {
    return this->ready;
}

const std::string NmapService::getReport() {
    return this->report;
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

void NmapService::scanTarget(const std::string &host, const std::vector<uint16_t> &ports)
{
    in_addr ip{};
    if (!resolveIPv4(host, ip)) {
        this->report.append("Failed to resolve host: ").append(host).append("\n");
        return;
    }

    char ipStr[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &ip, ipStr, sizeof(ipStr));
    this->report.append("Scanning host: ").append(host).append(" (").append(ipStr).append(")\n");

    for (uint16_t p : ports) {
        this->report.append("Scanning port ").append(std::to_string(p)).append("...\n");

        int st = tcp_connect_with_timeout(ip, p, CONNECT_TIMEOUT_MS);
        switch (st) {
            case nmap_rc_enum::TCP_OPEN:  this->report.append("Port ").append(std::to_string(p)).append("/tcp OPEN\n"); break;
            case nmap_rc_enum::TCP_CLOSED:  this->report.append("Port ").append(std::to_string(p)).append("/tcp CLOSED\n"); break;
            case nmap_rc_enum::TCP_FILTERED:  this->report.append("Port ").append(std::to_string(p)).append("/tcp FILTERED (timeout)\n"); break;
            default: this->report.append("Port ").append(std::to_string(p)).append("/tcp ERROR\n"); break;
        }

        // Yield
        vTaskDelay(pdMS_TO_TICKS(SMALL_DELAY_MS));
    }
}

void NmapService::clean()
{
    this->target_hosts.clear();
    this->target_ports.clear();
    this->report.clear();
    this->ready = false;
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

bool NmapService::parsePorts(const std::string& ports_arg)
{
    this->target_ports = std::vector<uint16_t>();

    // If we find ',' or '-' there are multiple ports
    if (ports_arg.find(',') != std::string::npos || ports_arg.find('-') != std::string::npos) {
        // Not yet implemented
        return false;
    }
    else {
        // Single port
        uint16_t port = static_cast<uint16_t>(std::stoi(ports_arg));
        this->target_ports.push_back(port);
    }
    return true;
}

bool NmapService::isIpv4(const std::string& address)
{
    // Check if the address is a valid IPv4 address
    struct sockaddr_in sa;
    return inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0;
}