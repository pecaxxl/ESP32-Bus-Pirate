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
    std::string targetIP;
    int count;
    int timeout_ms;
    int interval_ms;
    ICMPService *service;
};

ICMPService::ICMPService() {}

ICMPService::~ICMPService()
{
    cleanupICMPService();
}

void ICMPService::cleanupICMPService()
{
    // Reset last results
    ready = false;
    pingRC = ping_rc_t::ping_error;
    pingMedianMs = -1;
    pingTX = 0;
    pingRX = 0;
    report.clear();
}

static bool resolve_ipv4_to_ip_addr(const std::string &targetIP, ip_addr_t &out)
{
    // literal first
    ip4_addr_t a4{};
    if (ip4addr_aton(targetIP.c_str(), &a4))
    {
        out.type = IPADDR_TYPE_V4;
        out.u_addr.ip4 = a4;
        return true;
    }
    // DNS fallback
    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    struct addrinfo *res = nullptr;
    if (getaddrinfo(targetIP.c_str(), nullptr, &hints, &res) != 0 || !res)
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

void ICMPService::startDiscoveryTask(const std::string deviceIP)
{
    std::string deviceDiscoveryReport = "Discovery: scanning network for devices...\r\n";
    std::string deviceIPcopy = deviceIP;
    ip4_addr_t targetIP;
    TaskHandle_t pingTaskHandle = nullptr;
    uint32_t targetsResponded = 0;

    if (!ip4addr_aton(deviceIPcopy.c_str(), &targetIP))
    {
        report = "Discovery: failed to parse IP address " + deviceIP + "\r\n";
        return;
    }

    // Octets of an IPv4 addr
    uint8_t o1 = ip4_addr1(&targetIP);
    uint8_t o2 = ip4_addr2(&targetIP);
    uint8_t o3 = ip4_addr3(&targetIP);
    uint8_t deviceIndex = ip4_addr4(&targetIP);

    for (uint8_t targetIndex = 1; targetIndex < 255; targetIndex++)
    {
        if (targetIndex == deviceIndex)
            continue;

        // Rebuild the target IP from octets
        IP4_ADDR(&targetIP, o1, o2, o3, targetIndex);

        // Convert to dotted string
        char targetIPCStr[16];
        ip4addr_ntoa_r(&targetIP, targetIPCStr, sizeof(targetIPCStr));
        std::string targetIPStr(targetIPCStr);

        this->cleanupICMPService();
        auto *params = new ICMPTaskParams{targetIPStr, 1, 100, 100, this};
        xTaskCreatePinnedToCore(pingAPI, "ICMPPing", 4096, params, 1, &pingTaskHandle, 1);

        while (!this->ready)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        if (this->pingRC == ping_rc_t::ping_ok){
            deviceDiscoveryReport += "Device found: " + targetIPStr + "\r\n";
            targetsResponded++;
        }
        deviceDiscoveryReport.append(report);
    }

    deviceDiscoveryReport += std::to_string(targetsResponded) + " devices up, pinged " + 
        std::to_string(254 - targetsResponded) + " devices down\r\n";
    // Put all the results here, can be called with getReport
    report = std::move(deviceDiscoveryReport);
    ready = true;
}

void ICMPService::startPingTask(const std::string &targetIP, int count, int timeout_ms, int interval_ms)
{
    // Cleanup first
    this->cleanupICMPService();

    auto *params = new ICMPTaskParams{targetIP,
                                      count > 0 ? count : 5,
                                      timeout_ms > 0 ? timeout_ms : 1000,
                                      interval_ms > 0 ? interval_ms : 200,
                                      this};
    xTaskCreatePinnedToCore(pingAPI, "ICMPPing", 4096, params, 1, nullptr, 1);

    while(!this->ready)
        vTaskDelay(pdMS_TO_TICKS(10));

    // Prepare the report
    this->report.clear();
    if (this->pingRC == ping_rc_t::ping_ok || this->pingRC == ping_rc_t::ping_timeout) {
        this->report = "--- " + targetIP + " ping statistics ---\r\n";
        this->report += std::to_string(this->pingTX) + " packets transmitted, ";
        this->report += std::to_string(this->pingRX) + " received, ";
        this->report += std::to_string(this->pingRX * 100 / this->pingTX) + "\% packet loss,";
        this->report += " time " + std::to_string(this->pingMedianMs) + " ms\r\n";
    } else if (this->pingRC == ping_rc_t::ping_resolve_fail) {
        this->report = "Failed to resolve \"" + targetIP + "\"\r\n";
    } else if (this->pingRC == ping_rc_t::ping_session_fail) {
        this->report = "Failed to create session\r\n";
    } else {
        report = "Unknown error\r\n";
    }
        
}

void ICMPService::pingAPI(void *pvParams)
{
    auto *params = static_cast<ICMPTaskParams *>(pvParams);
    ICMPService *service = params->service;

    ip_addr_t target{};
    if (!resolve_ipv4_to_ip_addr(params->targetIP, target))
    {
        service->pingRC = ping_rc_t::ping_resolve_fail;
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
        service->pingRC = ping_rc_t::ping_session_fail;
        service->ready = true;
        delete params;
        vTaskDelete(nullptr);
        return;
    }

    esp_ping_start(h);

    // Wait up to count * (timeout + interval) + a small delay
    uint32_t wait_ms = (uint32_t)config.count * (config.timeout_ms + config.interval_ms) + 100;
    uint32_t t0 = millis();
    while (!ctx.done && (millis() - t0) < wait_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    esp_ping_stop(h);
    esp_ping_delete_session(h);

    // Results
    service->pingTX = (int)config.count;
    service->pingRX = (int)ctx.rx;
    service->pingRC = (service->pingRX > 0) ? ping_rc_t::ping_ok : ping_rc_t::ping_timeout;
    service->pingMedianMs = median_ms(ctx.rtts);

    if (service->pingRX > 0)
        service->pingRC = ping_rc_t::ping_ok;
    else if (service->pingRX == 0)
        service->pingRC = ping_rc_t::ping_timeout;

    service->ready = true;

    delete params;
    vTaskDelete(nullptr);
}
