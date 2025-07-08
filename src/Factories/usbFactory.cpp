#include "Factories/UsbFactory.h"

#if defined(DEVICE_CARDPUTER)
    #include <Services/UsbCardputerService.h>
    #include <Controllers/UsbCardputerController.h>
#endif

#if defined(DEVICE_M5STICK)
    #include <Services/UsbM5StickService.h>
    #include <Controllers/UsbM5StickController.h>
#endif

UsbComponents UsbFactory::create(ITerminalView& terminalView, IInput& terminalInput) {
#if defined(DEVICE_M5STICK)
    static ArgTransformer argTransformer;
    static UserInputManager userInputManager(terminalView, terminalInput, argTransformer);
    static UsbM5StickService usbService;
    static UsbM5StickController usbController(terminalView);

    return UsbComponents{
        argTransformer,
        userInputManager,
        usbService,
        usbController
    };

#elif defined(DEVICE_CARDPUTER)
    static ArgTransformer argTransformer;
    static UserInputManager userInputManager(terminalView, terminalInput, argTransformer);
    static UsbCardputerService usbService;
    static UsbCardputerController usbController(terminalView, terminalInput, usbService, argTransformer, userInputManager);

    return UsbComponents{
        argTransformer,
        userInputManager,
        usbService,
        usbController
    };

#else
    #error "No supported DEVICE defined for USBFactory"
#endif
}
