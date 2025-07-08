
#ifdef DEVICE_CARDPUTER

#include "UsbCardputerService.h"

UsbCardputerService::UsbCardputerService()
  : keyboardActive(false), storageActive(false), initialized(false) {}

void UsbCardputerService::keyboardBegin() {
    if (keyboardActive) return;

    USB.begin();
    keyboard.begin();
    hidInitTime = millis();
    keyboardActive = true;
}

void UsbCardputerService::storageBegin(uint8_t cs, uint8_t clk, uint8_t miso, uint8_t mosi) {
    if (initialized) return;

    sdSPI.begin(clk, miso, mosi, cs);
    if (!SD.begin(cs, sdSPI)) {
        storageActive = false;
        return;
    }

    // Setup MSC
    uint32_t secSize = SD.sectorSize();
    uint32_t numSectors = SD.numSectors();

    msc.vendorID("ESP32");
    msc.productID("USB_MSC");
    msc.productRevision("1.0");
    msc.onRead(storageReadCallback);
    msc.onWrite(storageWriteCallback);
    msc.onStartStop(usbStartStopCallback);
    msc.mediaPresent(true);
    msc.begin(numSectors, secSize);

    // Setup USB events
    setupStorageEvent();

    USB.begin();
    storageActive = true;
    initialized = true;
}

void UsbCardputerService::keyboardSendString(const std::string& text) {
    if (!keyboardActive) return;

    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    keyboard.releaseAll();
    for (char c : text) {
        keyboard.write(c);
    }
}

void UsbCardputerService::keyboardSendChunkedString(const std::string& data, size_t chunkSize, unsigned long delayBetweenChunks) {
    if (!keyboardActive) return;

    size_t totalLength = data.length();
    size_t sentLength = 0;

    while (sentLength < totalLength) {
        size_t currentChunkSize = std::min(chunkSize, totalLength - sentLength);
        std::string chunk = data.substr(sentLength, currentChunkSize);
        keyboardSendString(chunk);
        sentLength += currentChunkSize;
        delay(delayBetweenChunks);
    }
}

void UsbCardputerService::mouseBegin() {
    if (mouseActive) return;

    USB.begin();
    mouse.begin();
    mouseActive = true;
    hidInitTime = millis();
}

void UsbCardputerService::mouseMove(int x, int y) {
    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    mouse.move(x, y);
}

void UsbCardputerService::mouseClick(int button) {
    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    mouse.press(button);
    delay(50);
    mouse.release(button);
}

void UsbCardputerService::mouseRelease(int button) {
    if (!mouseActive) return;
    mouse.release(button);
}

void UsbCardputerService::gamepadBegin() {
    if (gamepadActive) return;

    USB.begin();
    gamepad.begin();
    gamepadActive = true;
    hidInitTime = millis();
}

void UsbCardputerService::gamepadPress(const std::string& name) {
    if (!gamepadActive) return;

    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    // Valeurs par défaut (repos)
    int8_t x = 0, y = 0, z = 0, rz = 0, rx = 0, ry = 0;
    uint8_t hat = HAT_CENTER;
    uint32_t buttons = 0;

    // Interprétation de la commande
    if (name == "up") {
        hat = HAT_UP;
    } else if (name == "down") {
        hat = HAT_DOWN;
    } else if (name == "left") {
        hat = HAT_LEFT;
    } else if (name == "right") {
        hat = HAT_RIGHT;
    } else if (name == "a") {
        buttons = (1 << BUTTON_A);
    } else if (name == "b") {
        buttons = (1 << BUTTON_B);
    } else {
        return; // commande inconnue
    }

    // Envoie de l'état
    gamepad.send(x, y, z, rz, rx, ry, hat, buttons);
    delay(100);  // Simule une pression courte
    gamepad.send(0, 0, 0, 0, 0, 0, HAT_CENTER, 0); // Relâche tout
}

bool UsbCardputerService::isKeyboardActive() const {
    return keyboardActive;
}

bool UsbCardputerService::isStorageActive() const {
    return storageActive;
}

bool UsbCardputerService::isMouseActive() const {
    return mouseActive;
}


int32_t UsbCardputerService::storageReadCallback(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0) return -1;

    uint8_t* buf = reinterpret_cast<uint8_t*>(buffer);
    for (uint32_t i = 0; i < bufsize / secSize; ++i) {
        if (!SD.readRAW(buf + i * secSize, lba + i)) {
            return -1;
        }
    }
    return bufsize;
}

int32_t UsbCardputerService::storageWriteCallback(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0) return -1;

    uint64_t freeSpace = SD.totalBytes() - SD.usedBytes();
    if (bufsize > freeSpace) {
        return -1;
    }

    for (uint32_t i = 0; i < bufsize / secSize; ++i) {
        uint8_t blk[secSize];
        memcpy(blk, buffer + i * secSize, secSize);
        if (!SD.writeRAW(blk, lba + i)) {
            return -1;
        }
    }
    return bufsize;
}

bool UsbCardputerService::usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject) {
    return true;
}

void UsbCardputerService::setupStorageEvent() {
    USB.onEvent([](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        if (event_base == ARDUINO_USB_EVENTS) {
            auto* data = reinterpret_cast<arduino_usb_event_data_t*>(event_data);
            switch (event_id) {
                case ARDUINO_USB_STARTED_EVENT:
                    // Started
                    break;
                case ARDUINO_USB_STOPPED_EVENT:
                    // Stopped
                    break;
                case ARDUINO_USB_SUSPEND_EVENT:
                    // Display a message or icon
                    break;
                case ARDUINO_USB_RESUME_EVENT:
                    // Display a message or icon
                    break;
                default:
                    break;
            }
        }
    });
}

void UsbCardputerService::reset() {
    if (initialized) {
        keyboard.releaseAll(); // si clavier actif
        msc.end();             // si stockage actif
        // USB.~ESPUSB();         // force la destruction de l'instance USB
        // USB.enableDFU();       // réinitialise le contrôleur USB (important)
        initialized = false;
        keyboardActive = false;
        storageActive = false;
        mouseActive = false;
    }
}

bool UsbCardputerService::isGamepadActive() const {
    return gamepadActive;
}


#endif