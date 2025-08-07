#include "NmapService.h"
#include <Arduino.h>
#include <lwip/sockets.h>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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