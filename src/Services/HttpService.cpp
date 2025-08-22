#include "HttpService.h"

struct HttpGetParams {
    std::string url;
    int timeout_ms;
    int bodyMaxBytes;
    bool insecure;
    HttpService* self;
};

void HttpService::startGetTask(const std::string& url, int timeout_ms, int bodyMaxBytes, bool insecure,
                               int stack_bytes, int core)
{
    ready = false;
    auto* p = new HttpGetParams{url, timeout_ms, bodyMaxBytes, insecure, this};
    xTaskCreatePinnedToCore(&HttpService::getTask, "HttpGet", stack_bytes/sizeof(StackType_t),
                            p, 1, nullptr, core);
}

void HttpService::getTask(void* pv)
{
    auto* p = static_cast<HttpGetParams*>(pv);
    auto* self = p->self;

    bool isHttps = (p->url.rfind("https://",0)==0);

    // Guard to prevent concurrent requests
    bool expected = false;
    if (!self->inFlight.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return; // already in flight
    }

    // Mark as not ready
    self->ready.store(false, std::memory_order_release);

    std::unique_ptr<WiFiClient> client;
    if (isHttps) {
        auto *c = new WiFiClientSecure();
        if (p->insecure) c->setInsecure();
        c->setTimeout(p->timeout_ms / 1000);
        client.reset(c);
    } else {
        auto *c = new WiFiClient();
        c->setTimeout(p->timeout_ms / 1000);
        client.reset(c);
    }

    HTTPClient http;
    http.setTimeout(p->timeout_ms);

    std::string result;

    if (!http.begin(*client, p->url.c_str())) {
        result = "ERROR: begin failed";
    } else {
        http.collectHeaders(HttpService::headerKeys,
                        sizeof(HttpService::headerKeys) / sizeof(HttpService::headerKeys[0]));

        // GET
        http.addHeader("Accept-Encoding", "identity");  // no gzip
        http.addHeader("Connection", "close"); // close socket
        int code = http.GET();

        // Check response code
        if (code > 0) {

            // Construct headers
            result = "HTTP/1.1 " + std::to_string(code) + "\r\n";
            int n = http.headers();
            for (int i=0; i<n; i++) {
                result += http.headerName(i).c_str();
                result += ": ";
                result += http.header(i).c_str();
                result += "\r\n";
            }

            // Json content type
            String ct = http.header("Content-Type");
            bool isJson = ct.length() && (ct.indexOf("json") >= 0);

            // If json, get the body
            if (isJson) {
                result += "\r\nJSON BODY:\n";
                result += HttpService::getJsonBody(http, p->bodyMaxBytes);
            }

        } else {
            result = "ERROR: " + std::string(http.errorToString(code).c_str());
        }
        http.getStream().stop();
        http.end();
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Ready to fetch the response
    self->response = result;
    self->ready.store(true, std::memory_order_release);
    self->inFlight.store(false, std::memory_order_release);
    delete p;
    vTaskDelete(nullptr);
}

std::string HttpService::getJsonBody(HTTPClient& http, int bodyMaxBytes) {
    std::string out;
    if (bodyMaxBytes <= 0) return out; // no length

    WiFiClient* stream = http.getStreamPtr();

    // Max bytes to read
    size_t budget = static_cast<size_t>(bodyMaxBytes);

    // Expected size if known, -1 if chunked/unknown
    int size = http.getSize();
    size_t target = (size > 0) ? (size_t)size : budget;
    if (size > 0 && target > budget) target = budget;

    // Small chunks to limit RAM spikes
    static constexpr size_t CHUNK = 256;
    uint8_t buf[CHUNK];

    // Small initial reserve (limits reallocs)
    out.reserve(std::min(target, (size_t)128));

    const unsigned long IDLE_TIMEOUT_MS = 1200;
    unsigned long lastDataMs = millis();
    size_t readTotal = 0;
    
    // Read condition
    auto canContinue = [&]() -> bool {
        // Continue as long as there is budget left and the socket is alive
        if (budget == 0) return false;
        if (stream->available() > 0) return true;
        // if content length is known, we can wait until everything is read
        if ((size < 0) && !stream->connected() && stream->available() == 0) return false;
        // idle timeout
        return (millis() - lastDataMs) < IDLE_TIMEOUT_MS;
    };
    
    // Read loop
    while (canContinue()) {
        int avail = stream->available();
        if (avail <= 0) { delay(1); continue; }

        size_t toRead = (size_t)avail;
        if (toRead > CHUNK)  toRead = CHUNK;
        if (toRead > budget) toRead = budget;

        int n = stream->read(buf, (int)toRead);
        if (n <= 0) { delay(1); continue; }

        size_t oldSz = out.size();
        out.resize(oldSz + (size_t)n);
        memcpy(&out[oldSz], buf, (size_t)n);

        readTotal += (size_t)n;
        budget    -= (size_t)n;
        lastDataMs = millis();

        if (size > 0 && readTotal >= (size_t)size) break; // all received
        if (budget == 0) {
            out += "...[TRUNCATED]";
            break; // budget exceeded
        }
    }
    return out;
}
