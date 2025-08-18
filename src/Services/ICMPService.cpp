#include "ICMPService.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct ICMPTaskParams
{
    int verbosity;
    ICMPService *service;
};

ICMPService::ICMPService() {
}

void ICMPService::startTask(int verbosity){
    // Start the ICMP task
    auto *params = new ICMPTaskParams{verbosity, this};
    xTaskCreatePinnedToCore(scanTask, "ICMPConnect", 20000, params, 1, nullptr, 1);
    delay(100); // start task delay
}

void ICMPService::scanTask(void *pvParams){
    auto *params = static_cast<ICMPTaskParams *>(pvParams);
    auto &service = *params->service;

    delete params;
    vTaskDelete(nullptr);
}