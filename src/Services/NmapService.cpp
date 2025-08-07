#include "NmapService.h"
#include <Arduino.h>
#include <lwip/sockets.h>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Data/NmapUtils.h>

struct NmapTaskParams
{
    std::string host;
    int verbosity;
    NmapService *service;
};

void NmapService::startTask(const std::string &host, int verbosity)
{
    auto *params = new NmapTaskParams{host, verbosity, this};
    //xTaskCreatePinnedToCore(connectTask, "NmapConnect", 20000, params, 1, nullptr, 1);
    delay(500); // start task delay
}

void NmapService::parseHosts(const std::string& hosts_arg)
{
    // Check if hosts_arg contains at least one letter (domain name)
    bool hasLetter = false;
    for (char c : hosts_arg) {
        if (isalpha(static_cast<unsigned char>(c))) {
            hasLetter = true;
            break;
        }
    }
    if (!hasLetter && !isIpv4(hosts_arg)) {
        // TODO: Handle invalid host
    }

    // TODO: Handle valid hosts
}

bool NmapService::isIpv4(const std::string& address)
{
    // Trick to check if the address is a valid IPv4 address
    struct sockaddr_in sa;
    return inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0;
}