#pragma once

/*
The DependencyProvider is responsible for creating, holding, 
and injecting shared instances of core components 
(such as services, controllers, etc) throughout the application.
*/

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IDeviceView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IUsbController.h"
#include "Interfaces/IUsbService.h"
#include "Services/SdService.h"
#include "Services/NvsService.h"
#include "Services/LedService.h"
#include "Services/UartService.h"
#include "Services/I2cService.h"
#include "Services/OneWireService.h"
#include "Services/InfraredService.h"
#include "Services/UsbS3Service.h"
#include "Services/HdUartService.h"
#include "Services/SpiService.h"
#include "Services/PinService.h"
#include "Services/BluetoothService.h"
#include "Services/WifiService.h"
#include "Services/I2sService.h"
#include "Services/SshService.h"
#include "Services/JtagService.h"
#include "Controllers/UartController.h"
#include "Controllers/I2cController.h"
#include "Controllers/OneWireController.h"
#include "Controllers/UtilityController.h"
#include "Controllers/UsbS3Controller.h"
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
#include "Controllers/I2sController.h"
#include "Transformers/TerminalCommandTransformer.h"
#include "Transformers/InstructionTransformer.h"
#include "Transformers/ArgTransformer.h"
#include "Transformers/WebRequestTransformer.h"
#include "Managers/CommandHistoryManager.h"
#include "Managers/BinaryAnalyzeManager.h"
#include "Managers/UserInputManager.h"
#include "Managers/SdCardManager.h"
#include "Config/TerminalTypeConfigurator.h"

class DependencyProvider {
public:
    DependencyProvider(ITerminalView& terminalView, IDeviceView& deviceView,
                       IInput& terminalInput, IInput& deviceInput,
                       IUsbService& usbService,
                       IUsbController& usbController);


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
    TwoWireService& getTwoWireService();
    InfraredService& getInfraredService();
    IUsbService& getUsbService();
    SpiService& getSpiService();
    HdUartService& getHdUartService();
    PinService& getPinService();
    BluetoothService& getBluetoothService();
    WifiService& getWifiService();
    I2sService& getI2sService();
    SshService& getSshService();
    JtagService& getJtagService();
    
    // Controllers
    UartController& getUartController();
    I2cController& getI2cController();
    UtilityController& getUtilityController();
    OneWireController& getOneWireController();
    InfraredController& getInfraredController();
    IUsbController& getUsbController();
    HdUartController& getHdUartController();
    SpiController& getSpiController();
    JtagController& getJtagController();
    TwoWireController& getTwoWireController();
    ThreeWireController& getThreeWireController();
    DioController& getDioController();
    LedController& getLedController();
    BluetoothController& getBluetoothController();
    WifiController& getWifiController();
    I2sController& getI2sController();
    
    // Transformers
    TerminalCommandTransformer& getCommandTransformer();
    InstructionTransformer& getInstructionTransformer();
    ArgTransformer& getArgTransformer();
    WebRequestTransformer& getWebRequestTransformer();

    // Managers
    CommandHistoryManager& getCommandHistoryManager();
    UserInputManager& getUserInputManager();
    BinaryAnalyzeManager& getBinaryAnalyzeManager();
    SdCardManager& getSdCardManager();

    // Selectors
    HorizontalSelector& getHorizontalSelector();

    // Config
    TerminalTypeConfigurator& getTerminalTypeConfigurator();

    // Disable
    void disableAllProtocols();

private:
    // Core Components
    ITerminalView& terminalView;
    IDeviceView& deviceView;
    IInput& terminalInput;
    IInput& deviceInput;
    IUsbService& usbService;
    IUsbController& usbController;
    
    // Services
    SdService sdService;
    NvsService nvsService;
    LedService ledService;
    UartService uartService;
    I2cService i2cService;
    OneWireService oneWireService;
    TwoWireService twoWireService;
    InfraredService infraredService;
    HdUartService hdUartService;
    SpiService spiService;
    PinService pinService;
    WifiService wifiService;
    BluetoothService bluetoothService;
    I2sService i2sService;
    SshService sshService;
    JtagService jtagService;

    // Controllers
    UartController uartController;
    I2cController i2cController;
    OneWireController oneWireController;
    UtilityController utilityController;
    InfraredController infraredController;
    HdUartController hdUartController;
    SpiController spiController;
    JtagController jtagController;
    TwoWireController twoWireController;
    ThreeWireController threeWireController;
    DioController dioController;
    LedController ledController;
    WifiController wifiController;
    BluetoothController bluetoothController;
    I2sController i2sController;

    // Transformers
    TerminalCommandTransformer commandTransformer;
    InstructionTransformer instructionTransformer;
    ArgTransformer argTransformer;
    WebRequestTransformer webRequestTransformer;

    // Managers
    CommandHistoryManager commandHistoryManager;
    UserInputManager userInputManager;
    BinaryAnalyzeManager binaryAnalyzeManager;
    SdCardManager sdCardManager;

    // Selectors
    HorizontalSelector horizontalSelector;

    // Config
    TerminalTypeConfigurator terminalTypeConfigurator;
};