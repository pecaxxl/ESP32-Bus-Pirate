#include "DependencyProvider.h"

DependencyProvider::DependencyProvider(ITerminalView& terminalView, IDeviceView& deviceView,
                                       IInput& terminalInput, IInput& deviceInput,
                                       IUsbService& usbService, IUsbController& usbController)
    : terminalView(terminalView),
      deviceView(deviceView),
      terminalInput(terminalInput),
      deviceInput(deviceInput),
      usbService(usbService),
      usbController(usbController),
      sdService(),
      nvsService(),
      ledService(),
      uartService(),
      i2cService(),
      oneWireService(),
      twoWireService(),
      infraredService(),
      spiService(),
      pinService(),
      bluetoothService(),
      wifiService(),
      i2sService(),
      commandTransformer(),
      instructionTransformer(),
      argTransformer(),
      webRequestTransformer(),
      commandHistoryManager(),
      binaryAnalyzeManager(spiService, terminalView, terminalInput),
      userInputManager(terminalView, terminalInput, argTransformer),
      horizontalSelector(deviceView, deviceInput),
      terminalTypeConfigurator(horizontalSelector),
      uartController(terminalView, terminalInput, deviceInput, uartService, sdService, hdUartService, argTransformer, userInputManager),
      i2cController(terminalView, terminalInput, i2cService, argTransformer, userInputManager),
      oneWireController(terminalView, terminalInput, oneWireService, argTransformer, userInputManager),
      infraredController(terminalView, terminalInput, infraredService, argTransformer, userInputManager),
      utilityController(terminalView, deviceView, terminalInput, pinService, userInputManager, argTransformer),
      hdUartController(terminalView, terminalInput, deviceInput, hdUartService, uartService, argTransformer, userInputManager),
      spiController(terminalView, terminalInput, spiService, sdService, argTransformer, userInputManager, binaryAnalyzeManager),
      jtagController(terminalView, terminalInput),
      twoWireController(terminalView, terminalInput, userInputManager, twoWireService),
      threeWireController(terminalView, terminalInput),
      dioController(terminalView, terminalInput, pinService, argTransformer),
      ledController(terminalView, terminalInput, ledService),
      bluetoothController(terminalView, terminalInput, bluetoothService, argTransformer),
      i2sController(terminalView, terminalInput, i2sService, argTransformer, userInputManager),
      wifiController(terminalView, terminalInput, wifiService, nvsService, argTransformer)
{}

// Accessors for core components
ITerminalView& DependencyProvider::getTerminalView() { return terminalView; }
void DependencyProvider::setTerminalView(ITerminalView& view) { terminalView = view; };
IDeviceView& DependencyProvider::getDeviceView() { return deviceView; }
IInput& DependencyProvider::getTerminalInput() { return terminalInput; }
IInput& DependencyProvider::getDeviceInput() { return deviceInput; }

// Services
SdService& DependencyProvider::getSdService() { return sdService; }
NvsService& DependencyProvider::getNvsService() { return nvsService; }
LedService& DependencyProvider::getLedService() { return ledService; }
I2cService& DependencyProvider::getI2cService() { return i2cService; }
UartService& DependencyProvider::getUartService() { return uartService; }
OneWireService& DependencyProvider::getOneWireService() { return oneWireService; }
TwoWireService& DependencyProvider::getTwoWireService() { return twoWireService; }
InfraredService& DependencyProvider::getInfraredService() { return infraredService; }
IUsbService& DependencyProvider::getUsbService() { return usbService; }
SpiService& DependencyProvider::getSpiService() { return spiService; }
HdUartService& DependencyProvider::getHdUartService() { return hdUartService; }
PinService& DependencyProvider::getPinService() { return pinService; }
WifiService& DependencyProvider::getWifiService() { return wifiService; }
BluetoothService& DependencyProvider::getBluetoothService() { return bluetoothService; }
I2sService& DependencyProvider::getI2sService() { return i2sService; }

// Controllers
UartController& DependencyProvider::getUartController() { return uartController; }
I2cController& DependencyProvider::getI2cController() { return i2cController; }
OneWireController& DependencyProvider::getOneWireController() { return oneWireController; }
UtilityController& DependencyProvider::getUtilityController() { return utilityController; }
InfraredController& DependencyProvider::getInfraredController() { return infraredController; }
IUsbController& DependencyProvider::getUsbController() { return usbController; }
HdUartController& DependencyProvider::getHdUartController() { return hdUartController; }
SpiController& DependencyProvider::getSpiController() { return spiController; }
JtagController& DependencyProvider::getJtagController() { return jtagController; }
TwoWireController& DependencyProvider::getTwoWireController() { return twoWireController; }
ThreeWireController& DependencyProvider::getThreeWireController() { return threeWireController; }
DioController& DependencyProvider::getDioController() { return dioController; }
LedController& DependencyProvider::getLedController() { return ledController; }
WifiController& DependencyProvider::getWifiController() { return wifiController; }
BluetoothController& DependencyProvider::getBluetoothController() { return bluetoothController; }
I2sController& DependencyProvider::getI2sController() { return i2sController; }

// Transformers
TerminalCommandTransformer& DependencyProvider::getCommandTransformer() { return commandTransformer; }
InstructionTransformer& DependencyProvider::getInstructionTransformer() { return instructionTransformer; }
ArgTransformer& DependencyProvider::getArgTransformer() { return argTransformer; }
WebRequestTransformer& DependencyProvider::getWebRequestTransformer() {return webRequestTransformer; }

// Managers
CommandHistoryManager& DependencyProvider::getCommandHistoryManager() { return commandHistoryManager ;}
UserInputManager& DependencyProvider::getUserInputManager() { return userInputManager; }
BinaryAnalyzeManager& DependencyProvider::getBinaryAnalyzeManager() { return binaryAnalyzeManager; }

// Selectors
HorizontalSelector& DependencyProvider::getHorizontalSelector() { return horizontalSelector ;}

// Config
TerminalTypeConfigurator& DependencyProvider::getTerminalTypeConfigurator() { return terminalTypeConfigurator ;}

// Disable interfaces
void DependencyProvider::disableAllProtocols() {
    getUartService().end();
    getHdUartService().end();
    getI2cService().end();
    getSpiService().end();
    getI2sService().end();
    getTwoWireService().end();
}
