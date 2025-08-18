#include "ICMPService.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>
#include <algorithm>

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "ping/ping_sock.h"
extern "C"
{
#include "esp_ping.h"
}

// Task params
struct ICMPTaskParams
{
    std::string host;
    int count;
    int timeout_ms;
    int interval_ms;
    ICMPService *service;
};

ICMPService::ICMPService() {}

ICMPService::~ICMPService() {
    cleanupICMPService();
}

void ICMPService::cleanupICMPService(){
    // Reset last results
    ready = false;
    ping_up = false;
    ping_median_ms = -1;
    ping_sent = 0;
    ping_recv = 0;
    report.clear();
}

static bool resolve_ipv4_to_ip_addr(const std::string &host, ip_addr_t &out)
{
    // literal first
    ip4_addr_t a4{};
    if (ip4addr_aton(host.c_str(), &a4))
    {
        out.type = IPADDR_TYPE_V4;
        out.u_addr.ip4 = a4;
        return true;
    }
    // DNS fallback
    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    struct addrinfo *res = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || !res)
        return false;
    out.type = IPADDR_TYPE_V4;
    out.u_addr.ip4.addr = ((sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(res);
    return true;
}

static int median_ms(std::vector<uint32_t> &v)
{
    if (v.empty())
        return -1;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n & 1)
        return (int)v[n / 2];
    return (int)((v[n / 2 - 1] + v[n / 2] + 1) / 2);
}

void ICMPService::startPingTask(const std::string &host, int count, int timeout_ms, int interval_ms)
{
    // Cleanup first
    this->cleanupICMPService();
    
    auto *params = new ICMPTaskParams{host,
                                      count > 0 ? count : 5,
                                      timeout_ms > 0 ? timeout_ms : 1000,
                                      interval_ms > 0 ? interval_ms : 200,
                                      this};
    xTaskCreatePinnedToCore(pingTask, "ICMPPing", 8192, params, 1, nullptr, 1);
    delay(10);
}

void ICMPService::pingTask(void *pvParams)
{
    auto *params = static_cast<ICMPTaskParams *>(pvParams);
    ICMPService *service = params->service;

    ip_addr_t target{};
    if (!resolve_ipv4_to_ip_addr(params->host, target))
    {
        service->report = "Ping: failed to resolve \"" + params->host + "\"\r\n";
        service->ready = true;
        delete params;
        vTaskDelete(nullptr);
        return;
    }

    // esp_ping setup
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr = target;  // destination
    config.count = params->count; // total probes
    config.interval_ms = params->interval_ms;
    config.timeout_ms = params->timeout_ms;

    struct Ctx
    {
        std::vector<uint32_t> rtts;
        volatile uint32_t rx = 0;
        volatile bool done = false;
    } ctx;

    esp_ping_callbacks_t cbs{};
    cbs.on_ping_success = [](esp_ping_handle_t h, void *arg)
    {
        uint32_t time_ms = 0;
        esp_ping_get_profile(h, ESP_PING_PROF_TIMEGAP, &time_ms, sizeof(time_ms)); // RTT
        auto *c = static_cast<Ctx *>(arg);
        c->rtts.push_back(time_ms);
        c->rx++;
    };
    cbs.on_ping_timeout = [](esp_ping_handle_t, void *)
    {
        // Count successes only for median
    };
    cbs.on_ping_end = [](esp_ping_handle_t, void *arg)
    {
        static_cast<Ctx *>(arg)->done = true;
    };
    cbs.cb_args = &ctx;

    esp_ping_handle_t h = nullptr;
    if (esp_ping_new_session(&config, &cbs, &h) != ESP_OK)
    {
        service->report = "Ping: failed to create session\r\n";
        service->ready = true;
        delete params;
        vTaskDelete(nullptr);
        return;
    }

    esp_ping_start(h);

    // Wait up to count * (timeout + interval) + a small delay
    uint32_t wait_ms = (uint32_t)config.count * (config.timeout_ms + config.interval_ms) + 500;
    uint32_t t0 = millis();
    while (!ctx.done && (millis() - t0) < wait_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    esp_ping_stop(h);
    esp_ping_delete_session(h);

    // Results
    service->ping_sent = (int)config.count;
    service->ping_recv = (int)ctx.rx;
    service->ping_up = (ctx.rx > 0);
    service->ping_median_ms = median_ms(ctx.rtts);

    char ipbuf[16] = {0};
    if (target.type == IPADDR_TYPE_V4)
    {
        ip4addr_ntoa_r(&target.u_addr.ip4, ipbuf, sizeof(ipbuf));
    }

    if (service->ping_up)
    {
        service->report = "Ping " + params->host + " (" + std::string(ipbuf) + ") UP, ";
        service->report += std::to_string(service->ping_recv) + "/" + std::to_string(service->ping_sent);
        service->report += " replies, median " + std::to_string(service->ping_median_ms) + " ms\r\n";
    }
    else
    {
        service->report = "Ping " + params->host + " (" + std::string(ipbuf) + ") DOWN, ";
        service->report += "0/" + std::to_string(service->ping_sent) + " replies\r\n";
    }

    service->ready = true;

    delete params;
    vTaskDelete(nullptr);
}
