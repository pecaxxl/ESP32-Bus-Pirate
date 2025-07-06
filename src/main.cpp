#ifndef UNIT_TEST

#include <M5Cardputer.h>
#include <Views/SerialTerminalView.h>
#include <Views/M5DeviceView.h>
#include <Views/WebTerminalView.h>
#include <Inputs/SerialTerminalInput.h>
#include <Inputs/CardputerInput.h>
#include <Providers/DependencyProvider.h>
#include <Dispatchers/ActionDispatcher.h>
#include <Servers/HttpServer.h>
#include <Servers/WebSocketServer.h>
#include <Inputs/WebTerminalInput.h>
#include <Selectors/HorizontalSelector.h>
#include <Config/TerminalTypeConfigurator.h>
#include <Config/WifiTypeConfigurator.h>
#include <Enums/TerminalTypeEnum.h>
#include <States/GlobalState.h>

void setup() {
    // Setup the cardputer
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);

    // Device View/Input
    M5DeviceView deviceView;
    CardputerInput deviceInput;
    deviceView.logo();

    // Select the terminal type
    HorizontalSelector selector(deviceView, deviceInput);
    TerminalTypeConfigurator configurator(selector);
    GlobalState& state = GlobalState::getInstance();
    TerminalTypeEnum terminalType = configurator.configure();

    switch (terminalType) {
        case TerminalTypeEnum::Serial: {
            // Serial View/Input
            SerialTerminalView serialView;
            SerialTerminalInput serialInput;
            
            // Baudrate
            auto baud = std::to_string(state.getSerialTerminalBaudRate());
            serialView.setBaudrate(state.getSerialTerminalBaudRate());

            // Build the provider for serial type and run the dispatcher lopp
            DependencyProvider provider(serialView, deviceView, serialInput, deviceInput);
            ActionDispatcher dispatcher(provider);
            dispatcher.setup(terminalType, baud);
            dispatcher.run();
            break;
        }
        case TerminalTypeEnum::WiFiAp: // Not Yet Implemented
        case TerminalTypeEnum::WiFiClient: {
            // Configure Wi-Fi mode
            WifiTypeConfigurator wifiTypeConfigurator;
            auto webIp = wifiTypeConfigurator.configure(terminalType);
            state.setTerminalIp(webIp);
            
            // Configure Server
            httpd_handle_t server = nullptr;
            httpd_config_t config = HTTPD_DEFAULT_CONFIG();
            if (httpd_start(&server, &config) != ESP_OK) {
                return;
            }
            HttpServer httpServer(server);
            WebSocketServer wsServer(server);

            // Web View/Input
            WebTerminalView webView(wsServer);
            WebTerminalInput webInput(wsServer);
            deviceView.loading();
            delay(3000); // let the server begin
            
            // Setup routes for index and ws
            httpServer.setupRoutes();
            wsServer.setupRoutes();

            // Build the provider for webui type and run the dispatcher loop
            DependencyProvider provider(webView, deviceView, webInput, deviceInput);
            ActionDispatcher dispatcher(provider);
            dispatcher.setup(terminalType, webIp);
            dispatcher.run();
            break;
        }
    }
}

void loop() {
    // Empty as all logic is handled in dispatcher
}

#endif