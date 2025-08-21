#include "HttpService.h"

static inline bool starts_with(const std::string& s, const char* p) {
    return s.rfind(p, 0) == 0;
}

esp_err_t HttpService::evt(esp_http_client_event_t *e) {
    if (!e || !e->user_data) return ESP_OK;
    auto* headers = reinterpret_cast<std::string*>(e->user_data);
    if (e->event_id == HTTP_EVENT_ON_HEADER && e->header_key && e->header_value) {
        headers->append(e->header_key);
        headers->append(": ");
        headers->append(e->header_value);
        headers->append("\r\n"); // CR+LF pour affichage propre
    }
    return ESP_OK;
}

std::string HttpService::get(const std::string& hostOrUrl, int timeout_ms) {
    // Force http:// if no prefix
    std::string url = hostOrUrl;
    if (!starts_with(url, "http://") && !starts_with(url, "https://")) {
        url = "https://" + url;
    }

    std::string headers;
    esp_http_client_config_t cfg = {};
    cfg.url = url.c_str();
    cfg.method = HTTP_METHOD_HEAD;
    cfg.timeout_ms = timeout_ms;
    cfg.disable_auto_redirect = false;
    cfg.max_redirection_count = 5;
    cfg.event_handler = &HttpService::evt;
    cfg.user_data = &headers;

    // // CA bundle selon l’environnement
    // esp_err_t arduino_esp_crt_bundle_attach(void *conf);
    // cfg.crt_bundle_attach = arduino_esp_crt_bundle_attach;

    // // Fallback DEV : HTTPS sans vérif (évite en prod)
    // cfg.transport_type = HTTP_TRANSPORT_OVER_SSL;
    // cfg.skip_cert_common_name_check = true;

    esp_http_client_handle_t h = esp_http_client_init(&cfg);
    if (!h) return "ERROR: esp_http_client_init failed";

    esp_err_t err = esp_http_client_perform(h);
    if (err != ESP_OK) {
        std::string out = "ERROR: ";
        out += esp_err_to_name(err);
        esp_http_client_cleanup(h);
        return out;
    }

    int status = esp_http_client_get_status_code(h);
    esp_http_client_cleanup(h);

    // Compose la chaîne finale
    std::string out = "HTTP/1.1 " + std::to_string(status) + "\r\n";
    out += headers;
    return out;
}
