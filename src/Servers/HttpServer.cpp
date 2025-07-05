#include "HttpServer.h"


HttpServer::HttpServer(httpd_handle_t sharedServer)
    : server(sharedServer) {}


void HttpServer::begin() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    if (httpd_start(&server, &config) == ESP_OK) {
        setupRoutes();
    }
}

void HttpServer::setupRoutes() {
    // Page HTML
    static httpd_uri_t root_uri;
    root_uri.uri = "/";
    root_uri.method = HTTP_GET;
    root_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleRootRequest(req);
    };
    root_uri.user_ctx = this;
    httpd_register_uri_handler(server, &root_uri);

    // CSS
    static httpd_uri_t css_uri;
    css_uri.uri = "/style.css";
    css_uri.method = HTTP_GET;
    css_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleCssRequest(req);
    };
    css_uri.user_ctx = this;
    httpd_register_uri_handler(server, &css_uri);

    // JavaScript
    static httpd_uri_t js_uri;
    js_uri.uri = "/scripts.js";
    js_uri.method = HTTP_GET;
    js_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleJsRequest(req);
    };
    js_uri.user_ctx = this;
    httpd_register_uri_handler(server, &js_uri);

    // Route test ping
    static httpd_uri_t ping_uri;
    ping_uri.uri = "/ping";
    ping_uri.method = HTTP_GET;
    ping_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        return httpd_resp_send(req, "pong", HTTPD_RESP_USE_STRLEN);
    };
    ping_uri.user_ctx = nullptr;
    httpd_register_uri_handler(server, &ping_uri);
    
}

esp_err_t HttpServer::handleRootRequest(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char*)index_html, strlen((const char*)index_html));
}

esp_err_t HttpServer::handleCssRequest(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css");
    return httpd_resp_send(req, (const char*)style_css, strlen((const char*)style_css));
}

esp_err_t HttpServer::handleJsRequest(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/javascript");
    return httpd_resp_send(req, (const char*)scripts_js, strlen((const char*)scripts_js));
}