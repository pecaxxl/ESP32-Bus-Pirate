#pragma once

/*
The DependencyProvider is responsible for creating, holding, 
and injecting shared instances of core components 
(such as services, controllers, etc) throughout the application.
*/

#include "Views/ITerminalView.h"
#include "Views/IDeviceView.h"
#include "Inputs/IInput.h"
#include "Services/SdService.h"
#include "Services/NvsService.h"
#include "Services/LedService.h"
#include "Services/UartService.h"
#include "Services/I2cService.h"
#include "Services/OneWireService.h"
#include "Services/InfraredService.h"
#include "Services/UsbService.h"
#include "Services/HdUartService.h"
#include "Services/SpiService.h"
#include "Services/PinService.h"
#include "Services/BluetoothService.h"
#include "Services/WifiService.h"
#include "Controllers/UartController.h"
#include "Controllers/I2cController.h"
#include "Controllers/OneWireController.h"
#include "Controllers/UtilityController.h"
#include "Controllers/UsbController.h"
#include "Controllers/HdUartController.h"
#include "Controllers/JtagController.h"
#include "Controllers/SpiController.h"
#include "Controllers/TwoWireController.h"
#include "Controllers/ThreeWireController.h"
#include "Controllers/InfraredController.h"
#include "Controllers/DioController.h"
#include "Controllers/LedController.h"
#include "Controllers/BluetoothController.h"
#include "Controllers/WifiController.h"
#include "Transformers/TerminalCommandTransformer.h"
#include "Transformers/InstructionTransformer.h"
#include "Transformers/ArgTransformer.h"
#include "Transformers/WebRequestTransformer.h"
#include "Managers/CommandHistoryManager.h"
#include "Managers/UserInputManager.h"
#include "Config/TerminalTypeConfigurator.h"


class DependencyProvider {
public:
    DependencyProvider(ITerminalView& terminalView, IDeviceView& deviceView, 
                       IInput& terminalInput, IInput& deviceInput);

    // Core Components
    ITerminalView& getTerminalView();
    void setTerminalView(ITerminalView& view);
    IDeviceView& getDeviceView();
    IInput& getTerminalInput();
    IInput& getDeviceInput();

    // Services
    SdService& getSdService();
    NvsService& getNvsService();
    LedService& getLedService();
    UartService& getUartService();
    I2cService& getI2cService();
    OneWireService& getOneWireService();
    InfraredService& getInfraredService();
    UsbService& getUsbService();
    SpiService& getSpiService();
    HdUartService& getHdUartService();
    PinService& getPinService();
    BluetoothService& getBluetoothService();
    WifiService& getWifiService();

    // Controllers
    UartController& getUartController();
    I2cController& getI2cController();
    UtilityController& getUtilityController();
    OneWireController& getOneWireController();
    InfraredController& getInfraredController();
    UsbController& getUsbController();
    HdUartController& getHdUartController();
    SpiController& getSpiController();
    JtagController& getJtagController();
    TwoWireController& getTwoWireController();
    ThreeWireController& getThreeWireController();
    DioController& getDioController();
    LedController& getLedController();
    BluetoothController& getBluetoothController();
    WifiController& getWifiController();

    // Transformers
    TerminalCommandTransformer& getCommandTransformer();
    InstructionTransformer& getInstructionTransformer();
    ArgTransformer& getArgTransformer();
    WebRequestTransformer& getWebRequestTransformer();

    // Managers
    CommandHistoryManager& getCommandHistoryManager();
    UserInputManager& getUserInputManager();

    // Selectors
    HorizontalSelector& getHorizontalSelector();

    // Config
    TerminalTypeConfigurator& getTerminalTypeConfigurator();

private:
    // Core Components
    ITerminalView& terminalView;
    IDeviceView& deviceView;
    IInput& terminalInput;
    IInput& deviceInput;
    
    // Services
    SdService sdService;
    NvsService nvsService;
    LedService ledService;
    UartService uartService;
    I2cService i2cService;
    OneWireService oneWireService;
    InfraredService infraredService;
    UsbService usbService;
    HdUartService hdUartService;
    SpiService spiService;
    PinService pinService;
    WifiService wifiService;
    BluetoothService bluetoothService;

    // Controllers
    UartController uartController;
    I2cController i2cController;
    OneWireController oneWireController;
    UtilityController utilityController;
    InfraredController infraredController;
    UsbController usbController;
    HdUartController hdUartController;
    SpiController spiController;
    JtagController jtagController;
    TwoWireController twoWireController;
    ThreeWireController threeWireController;
    DioController dioController;
    LedController ledController;
    WifiController wifiController;
    BluetoothController bluetoothController;
    
    // Transformers
    TerminalCommandTransformer commandTransformer;
    InstructionTransformer instructionTransformer;
    ArgTransformer argTransformer;
    WebRequestTransformer webRequestTransformer;

    // Managers
    CommandHistoryManager commandHistoryManager;
    UserInputManager userInputManager;

    // Selectors
    HorizontalSelector horizontalSelector;

    // Config
    TerminalTypeConfigurator terminalTypeConfigurator;
};