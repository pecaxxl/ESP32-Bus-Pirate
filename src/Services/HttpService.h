#pragma once

#undef INADDR_NONE // conflicting
#undef IPADDR_NONE

#include <HTTPClient.h>
#include <vector>
#include <string>
#include <atomic>

class HttpService {
public:
    // ============ GET ================

    // Http(s) get task on url
    void startGetTask(const std::string& url,
                      int timeout_ms,
                      int bodyMaxBytes,
                      bool insecure,
                      int stack_bytes = 24 * 1024,
                      int core = 1);

    // ===================================

    // Verify if the response is ready
    bool isResponseReady() const noexcept;

    // Get last HTTP response, status + headers + (json body)
    std::string lastResponse();

    // Resets the internal state
    void reset() { response.clear(); ready = false; }

private:
    static void getTask(void* pv);
    static std::string getJsonBody(HTTPClient& http, int bodyMaxBytes);
    void ensureClient(bool https, bool insecure, int timeout_s);
    bool beginHttp(const std::string& url, int timeout_ms);

    // Response
    std::atomic<bool> ready{false}; // response is ready
    std::string response; // http response

    // HTTP client
    std::unique_ptr<WiFiClient> client_;
    bool client_https_ = false;
    bool client_inited_ = false;
    HTTPClient http_;

    inline static const char* headerKeys[] = {
      // General
      "Location","Server","Date","Content-Type","Content-Length",
      "Connection","Keep-Alive","Cache-Control","Expires","Last-Modified",
      "ETag","Vary","Transfer-Encoding","Content-Encoding","Content-Language",
      "Content-Disposition","Accept-Ranges","Content-Range","Set-Cookie",
      // auth / quotas
      "WWW-Authenticate","Retry-After",
      "X-RateLimit-Limit","X-RateLimit-Remaining","X-RateLimit-Reset",
      // CORS
      "Access-Control-Allow-Origin","Access-Control-Allow-Credentials",
      "Access-Control-Allow-Methods","Access-Control-Allow-Headers",
      "Access-Control-Expose-Headers","Access-Control-Max-Age",
      // Security
      "Strict-Transport-Security","Content-Security-Policy",
      "X-Content-Type-Options","X-Frame-Options","Referrer-Policy",
      "Permissions-Policy",
      // perfs
      "Alt-Svc","Server-Timing"
    };

};
