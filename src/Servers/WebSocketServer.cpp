#include "WebSocketServer.h"
#include <esp_log.h>
#include <esp_http_server.h>
#include <cstring>

static const char* TAG = "WebSocketServer";

WebSocketServer::WebSocketServer(httpd_handle_t sharedServer)
    : server(sharedServer) {}

void WebSocketServer::setupRoutes() {
    static httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WebSocketServer::wsHandler,
        .user_ctx = this,
        .is_websocket = true
    };

    httpd_register_uri_handler(server, &ws_uri);
}


void WebSocketServer::begin() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    if (httpd_start(&server, &config) == ESP_OK) {
        setupRoutes();
    }
}

esp_err_t WebSocketServer::wsHandler(httpd_req_t *req) {
    WebSocketServer* self = static_cast<WebSocketServer*>(req->user_ctx);
    
    if (req->method == HTTP_GET) {
        clientFd = httpd_req_to_sockfd(req);  // Capture le socket client
        return ESP_OK;
    }
    
    
    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = nullptr;

    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    frame.payload = (uint8_t*)malloc(frame.len + 1);
    if (!frame.payload) return ESP_ERR_NO_MEM;

    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK) {
        free(frame.payload);
        return ret;
    }
    frame.payload[frame.len] = '\0';
    
    // Push chars one by one into buffer
    for (size_t i = 0; i < frame.len; ++i) {
        self->buffer.push_back(((char*)frame.payload)[i]);
    }

    free(frame.payload);
    return ESP_OK;
}

char WebSocketServer::readCharBlocking() {
    while (buffer.empty()) {
        delay(10);
    }
    char c = buffer.front();
    buffer.pop_front();
    return c;
}

char WebSocketServer::readCharNonBlocking() {
    const unsigned long timeout = millis() + 20; // some time for websocket

    while (buffer.empty() && millis() < timeout) {
        delay(1);  // laisse tourner la loop
    }

    if (buffer.empty()) return KEY_NONE;

    char c = buffer.front();
    buffer.pop_front();

    return c;
}

void WebSocketServer::sendText(const std::string& msg) {
    if (clientFd < 0) return;

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*) msg.c_str();
    ws_pkt.len = msg.length();

    esp_err_t ret = httpd_ws_send_frame_async(server, clientFd, &ws_pkt);
}
