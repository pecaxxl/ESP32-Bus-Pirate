#ifndef UNIT_TEST

#include <M5Unified.h>
#include <Views/SerialTerminalView.h>
#include <Views/M5DeviceView.h>
#include <Views/WebTerminalView.h>
#include <Inputs/SerialTerminalInput.h>
#include <Inputs/CardputerInput.h>
#include <Inputs/StickInput.h>
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
#include <Factories/UsbFactory.h>


void setup() {
    
    M5DeviceView deviceView;
    GlobalState& state = GlobalState::getInstance();
    
    #if defined(DEVICE_M5STICK)
        // Setup the M5stickCplus2
        auto cfg = M5.config();
        M5.begin(cfg);
        deviceView.setRotation(3);
        StickInput deviceInput;
    #else
        // Setup the Cardputer
        auto cfg = M5.config();
        M5Cardputer.begin(cfg, true);
        deviceView.setRotation(1);
        CardputerInput deviceInput;
    #endif

    deviceView.logo();
    
    // Select the terminal type
    HorizontalSelector selector(deviceView, deviceInput);
    TerminalTypeConfigurator configurator(selector);
    TerminalTypeEnum terminalType = configurator.configure();
    std::string webIp = "0.0.0.0";

    // Configure Wi-Fi if needed
    if (terminalType == TerminalTypeEnum::WiFiClient) {
        WifiTypeConfigurator wifiTypeConfigurator;
        webIp = wifiTypeConfigurator.configure(terminalType);
        
        if (webIp == "0.0.0.0") {
            terminalType = TerminalTypeEnum::Serial;
        } else {
            state.setTerminalIp(webIp);
        }
    }

    switch (terminalType) {
        case TerminalTypeEnum::Serial: {
            // Serial View/Input
            SerialTerminalView serialView;
            SerialTerminalInput serialInput;
            
            // Baudrate
            auto baud = std::to_string(state.getSerialTerminalBaudRate());
            serialView.setBaudrate(state.getSerialTerminalBaudRate());

            // Configure USB
            auto usb = UsbFactory::create(serialView, serialInput);

            // Build the provider for serial type and run the dispatcher lopp
            DependencyProvider provider(serialView, deviceView, serialInput, deviceInput, usb.usbService, usb.usbController);
            ActionDispatcher dispatcher(provider);
            dispatcher.setup(terminalType, baud);
            dispatcher.run();
            break;
        }
        case TerminalTypeEnum::WiFiAp: // Not Yet Implemented
        case TerminalTypeEnum::WiFiClient: {
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
            delay(5000); // let the server begin
            
            // Setup routes for index and ws
            httpServer.setupRoutes();
            wsServer.setupRoutes();

            // Configure USB
            auto usb = UsbFactory::create(webView, webInput);

            // Build the provider for webui type and run the dispatcher loop
            DependencyProvider provider(webView, deviceView, webInput, deviceInput, usb.usbService, usb.usbController);
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