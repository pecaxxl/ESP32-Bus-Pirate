#include "NmapService.h"
#include <Arduino.h>
#include <lwip/sockets.h>
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

void NmapService::scanTarget(const std::string &host, const std::vector<uint16_t> &ports)
{

}

void NmapService::connectTask(void *pvParams)
{
    auto *params = static_cast<NmapTaskParams *>(pvParams);
    //params->service->connect(params->target_hosts, params->verbosity, params->target_ports);
    //delete params;
    vTaskDelete(nullptr);
}

void NmapService::startTask(int verbosity)
{
    auto *params = new NmapTaskParams{this->target_hosts, this->target_ports, verbosity, this};
    xTaskCreatePinnedToCore(connectTask, "NmapConnect", 20000, params, 1, nullptr, 1);
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
    // Trick to check if the address is a valid IPv4 address
    struct sockaddr_in sa;
    return inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0;
}