#ifndef UNIT_TEST

#include <Views/SerialTerminalView.h>
#include <Views/M5DeviceView.h>
#include <Views/WebTerminalView.h>
#include <Views/NoScreenDeviceView.h>
#include <Views/TembedDeviceView.h>
#include <Inputs/SerialTerminalInput.h>
#include <Inputs/CardputerInput.h>
#include <Inputs/StickInput.h>
#include <Inputs/StampS3Input.h>
#include <Inputs/TembedInput.h>
#include <Inputs/S3DevKitInput.h>
#include <Providers/DependencyProvider.h>
#include <Dispatchers/ActionDispatcher.h>
#include <Servers/HttpServer.h>
#include <Servers/WebSocketServer.h>
#include <Inputs/WebTerminalInput.h>
#include <Selectors/HorizontalSelector.h>
#include <Config/TerminalTypeConfigurator.h>
#include <Config/WifiTypeConfigurator.h>
#include <Config/UsbConfigurator.h>
#include <Enums/TerminalTypeEnum.h>
#include <States/GlobalState.h>

void setup() {    
    #if DEVICE_M5STICK
        // Setup the M5stickCplus2
        #include <M5Unified.h>
        auto cfg = M5.config();
        M5.begin(cfg);
        M5DeviceView deviceView;
        deviceView.setRotation(3);
        StickInput deviceInput;
    #elif DEVICE_CARDPUTER
        // Setup the Cardputer
        #include <M5Unified.h>
        auto cfg = M5.config();
        M5Cardputer.begin(cfg, true);
        M5DeviceView deviceView;
        deviceView.setRotation(1);
        CardputerInput deviceInput;
    #elif DEVICE_M5STAMPS3
        // Setup the StampS3/AtomS3
        #include <M5Unified.h>
        auto cfg = M5.config();
        M5.begin(cfg);
        NoScreenDeviceView deviceView;
        StampS3Input deviceInput;
    #elif defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)
        // Setup the T-embed
        TembedDeviceView deviceView;
        TembedInput deviceInput;
    #else
        // Fallback to S3 dev kit
        NoScreenDeviceView deviceView;
        S3DevKitInput deviceInput;
    #endif

    deviceView.logo();
    GlobalState& state = GlobalState::getInstance();

    // Select the terminal type
    HorizontalSelector selector(deviceView, deviceInput);
    TerminalTypeConfigurator configurator(selector);
    TerminalTypeEnum terminalType = configurator.configure();
    std::string webIp = "0.0.0.0";

    // Configure Wi-Fi if needed
    if (terminalType == TerminalTypeEnum::WiFiClient) {
        WifiTypeConfigurator wifiTypeConfigurator(deviceView, deviceInput);
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
            auto usb = UsbConfigurator::configure(serialView, serialInput, deviceInput);

            // Build the provider for serial type and run the dispatcher loop
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
            auto usb = UsbConfigurator::configure(webView, webInput, deviceInput);

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