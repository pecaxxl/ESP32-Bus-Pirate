#include "HttpService.h"

struct HttpGetParams {
    std::string url;
    int timeout_ms;
    int bodyMaxBytes;
    bool insecure;
    HttpService* self;
};

void HttpService::ensureClient(bool https, bool insecure, int timeout_s)
{
    // Recreate the client if not initialized or if the type changes
    if (!client_inited_ || client_https_ != https) {
        client_.reset(); // destroy the old one properly
        if (https) {
            auto* c = new WiFiClientSecure();
            if (insecure) static_cast<WiFiClientSecure*>(c)->setInsecure();
            client_.reset(c);
        } else {
            auto* c = new WiFiClient();
            client_.reset(c);
        }
        client_https_ = https;
        client_inited_ = true;
    } else {
        // Reuse
        if (https) {
            auto* c = static_cast<WiFiClientSecure*>(client_.get());
            if (insecure) c->setInsecure();
        }
    }
}

bool HttpService::beginHttp(const std::string& url, int timeout_ms)
{
    http_.setTimeout(timeout_ms);
    http_.setReuse(false);

    return http_.begin(*client_, url.c_str());
}

void HttpService::startGetTask(const std::string& url, int timeout_ms, int bodyMaxBytes, bool insecure,
                               int stack_bytes, int core)
{
    ready = false;
    auto* p = new HttpGetParams{url, timeout_ms, bodyMaxBytes, insecure, this};
    xTaskCreatePinnedToCore(&HttpService::getTask, "HttpGet", stack_bytes,
                            p, 1, nullptr, core);
}

void HttpService::getTask(void* pv)
{
    auto* p = static_cast<HttpGetParams*>(pv);
    auto* self = p->self;
    self->ready.store(false, std::memory_order_relaxed);

    // Determine if the request is HTTPS
    const bool isHttps = (p->url.rfind("https://", 0) == 0);

    // Reuse client
    self->ensureClient(isHttps, p->insecure, p->timeout_ms / 1000);
    std::string result;

    // Start HTTP
    if (!self->beginHttp(p->url, p->timeout_ms)) {
        result = "ERROR: begin failed";
        self->http_.getStream().stop();
        self->http_.end();
        delete p;
        vTaskDelete(nullptr);
        return;
    }

    // Headers
    self->http_.collectHeaders(HttpService::headerKeys,
                               sizeof(HttpService::headerKeys) / sizeof(HttpService::headerKeys[0]));
    self->http_.addHeader("Accept-Encoding", "identity");
    self->http_.addHeader("Connection", "close");

    // Send GET request
    const int code = self->http_.GET();
    if (code > 0) {
        // Response headers
        result = "HTTP/1.1 " + std::to_string(code) + "\r\n";
        const int n = self->http_.headers();
        for (int i = 0; i < n; i++) {
            result += self->http_.headerName(i).c_str();
            result += ": ";
            result += self->http_.header(i).c_str();
            result += "\r\n";
        }

        // Json body if content type is json
        const String ct = self->http_.header("Content-Type");
        const bool isJson = ct.length() && (ct.indexOf("json") >= 0);
        if (isJson) {
            result += "\r\nJSON BODY:\n";
            result += HttpService::getJsonBody(self->http_, p->bodyMaxBytes);
        } 
    } else {
        result = "ERROR: ";
        result += self->http_.errorToString(code).c_str();
    }

    // Clean HTTP
    self->http_.getStream().stop();
    self->http_.end();
    vTaskDelay(pdMS_TO_TICKS(10));

    // Ready to get response
    self->response = std::move(result);
    self->ready.store(true, std::memory_order_release);

    delete p;
    vTaskDelete(nullptr);
}

std::string HttpService::getJsonBody(HTTPClient& http, int bodyMaxBytes)
{
    std::string out;
    if (bodyMaxBytes <= 0) return out;

    WiFiClient* stream = http.getStreamPtr();
    size_t budget = static_cast<size_t>(bodyMaxBytes);

    int size = http.getSize();
    size_t target = (size > 0) ? (size_t)size : budget;
    if (size > 0 && target > budget) target = budget;

    static constexpr size_t CHUNK = 256;
    uint8_t buf[CHUNK];

    out.reserve(std::min(target, (size_t)128));

    const unsigned long IDLE_TIMEOUT_MS = 1200;
    unsigned long lastDataMs = millis();
    size_t readTotal = 0;

    auto canContinue = [&]() -> bool {
        if (budget == 0) return false;
        if (stream->available() > 0) return true;
        if ((size < 0) && !stream->connected() && stream->available() == 0) return false;
        return (millis() - lastDataMs) < IDLE_TIMEOUT_MS;
    };

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

        if (size > 0 && readTotal >= (size_t)size) break;
        if (budget == 0) {
            out += "...[TRUNCATED]";
            break;
        }
    }
    return out;
}

std::string HttpService::lastResponse()
{
    std::string out;
    response.swap(out);
    ready.store(false, std::memory_order_release);
    return out;
}

bool HttpService::isResponseReady() const noexcept {
    return ready.load(std::memory_order_acquire);
}
