#pragma once

#include <Arduino.h>
#include <cstring>
#include <string>
#include <esp_http_server.h>
#include "../webui/index.h"
#include "../webui/scripts.h"
#include "../webui/style.h"

class HttpServer {
public:
    HttpServer(httpd_handle_t sharedServer);
    void begin();
    void setupRoutes();

private:
    httpd_handle_t server;
    esp_err_t handleRootRequest(httpd_req_t *req);
    esp_err_t handleCssRequest(httpd_req_t *req);
    esp_err_t handleJsRequest(httpd_req_t *req);
};
