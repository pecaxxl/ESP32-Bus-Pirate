#include "BluetoothService.h"

class BluetoothServerCallbacks : public BLEServerCallbacks {
private:
    BluetoothService& service;

public:
    BluetoothServerCallbacks(BluetoothService& svc) : service(svc) {}

    void onConnect(BLEServer* pServer) override {
        service.onConnect();
    }

    void onDisconnect(BLEServer* pServer) override {
        service.onDisconnect();
        pServer->startAdvertising();
    }
};

void BluetoothService::begin(const std::string& deviceName) {
    end();
    delay(200);
    BLEDevice::init(deviceName);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BluetoothServerCallbacks(*this));

    hid = new BLEHIDDevice(server);
    mouseInput = hid->inputReport(1);
    keyboardInput = hid->inputReport(2);

    hid->manufacturer()->setValue("M5Stack");
    hid->pnp(0x02, 0x1234, 0x5678, 0x0100);
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t*)HID_REPORT_MAP, 117);
    hid->startServices();

    BLEAdvertising* advertising = server->getAdvertising();
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->start();

    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);
    security->setCapability(ESP_IO_CAP_NONE);
    security->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    mode = BluetoothMode::SERVER;
    connected = true;
}

void BluetoothService::end() {
    if (hid) {
        delete hid;
        hid = nullptr;
    }

    mouseInput = nullptr;
    keyboardInput = nullptr;

    if (BLEDevice::getInitialized()) {
        BLEDevice::deinit();
        delay(100);
    }


    connected = false;
    mode = BluetoothMode::NONE;
}

void BluetoothService::onConnect() {
    connected = true;
}

void BluetoothService::onDisconnect() {
    connected = false;
}

bool BluetoothService::isConnected() const {
    return connected;
}

void BluetoothService::mouseMove(int16_t x, int16_t y) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !mouseInput) return;
    sendMouseReport(x, y, 0x0);
}

void BluetoothService::sendKeyboardReport(uint8_t modifier, const std::array<uint8_t, 6>& keys) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !keyboardInput) return;
    uint8_t report[8] = {modifier, 0, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]};
    keyboardInput->setValue(report, sizeof(report));
    keyboardInput->notify();
}

void BluetoothService::sendKeyboardText(const std::string& text) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !keyboardInput) return;

    for (char c : text) {
        if (c < 0 || c > 127) continue;

        AsciiHid entry = asciiHid[(uint8_t)c];
        if (entry.keycode == 0) continue;

        uint8_t modifier = entry.requiresShift ? 0x02 : 0x00;  // Shift

        std::array<uint8_t, 6> keys = {0};
        keys[0] = entry.keycode;

        sendKeyboardReport(modifier, keys);
        delay(10);

        std::array<uint8_t, 6> emptyKeys = {0};
        sendKeyboardReport(0, emptyKeys);
        delay(10);
    }
}

std::string BluetoothService::getMacAddress() {
    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_BT) != ESP_OK) {
        return "Unavailable";
    }

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return std::string(macStr);
}

void BluetoothService::sendEmptyReports() {
    if (mode != BluetoothMode::SERVER) return;
    if (mouseInput) {
        uint8_t emptyMouseReport[4] = {0, 0, 0, 0};
        mouseInput->setValue(emptyMouseReport, sizeof(emptyMouseReport));
        mouseInput->notify();
    }
    if (keyboardInput) {
        uint8_t emptyKeyboardReport[8] = {0};
        keyboardInput->setValue(emptyKeyboardReport, sizeof(emptyKeyboardReport));
        keyboardInput->notify();
    }
}

void BluetoothService::pairWithAddress(const std::string& addrStr) {
    BLEAddress addr(addrStr);
    BLEDevice::whiteListAdd(addr);  // Optionnel ?
}

void BluetoothService::sendMouseReport(int16_t x, int16_t y, uint8_t buttons) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !mouseInput) return;
    uint8_t report[4] = {buttons, (uint8_t)x, (uint8_t)y, 0};
    mouseInput->setValue(report, sizeof(report));
    mouseInput->notify();
}

void BluetoothService::clickMouse() {
    sendMouseReport(0, 0, 0x01);  // Left click
    delay(50);
    sendMouseReport(0, 0, 0x00);  // Release
}

std::vector<ScannedDevice> BluetoothService::scanDevices(int seconds) {
    auto scan = BLEDevice::getScan();
    scan->setActiveScan(true);

    auto results = scan->start(seconds);
    std::vector<ScannedDevice> devices;

    for (int i = 0; i < results.getCount(); ++i) {
        BLEAdvertisedDevice device = results.getDevice(i);
        devices.push_back({
            device.getName().empty() ? "(unknown)" : device.getName(),
            device.getAddress().toString(),
            device.getRSSI()
        });
    }

    scan->clearResults();
    return devices;
}

std::vector<std::string> BluetoothService::connectTo(const std::string& addr) {
    std::vector<std::string> serviceUUIDs;

    if (mode != BluetoothMode::CLIENT) {
        BLEDevice::init("BLE-Client");
        mode = BluetoothMode::CLIENT;
    }

    BLEAddress address(addr);
    BLEClient* client = BLEDevice::createClient();

    if (!client->connect(address)) {
        return serviceUUIDs;
    }

    auto* services = client->getServices();
    if (services) {
        for (const auto& pair : *services) {
            serviceUUIDs.push_back(pair.first);
        }
    }

    client->disconnect();
    return serviceUUIDs;
}

void BluetoothService::init(const std::string& deviceName) {
    if (mode == BluetoothMode::CLIENT && BLEDevice::getInitialized()) {
        // Already init
        return;
    }

    BLEDevice::init(deviceName);
    mode = BluetoothMode::CLIENT;
}

BluetoothMode BluetoothService::getMode() {
    return mode;
}

bool BluetoothService::spoofMacAddress(const std::string& macStr) {
    if (BLEDevice::getInitialized()) {
        return false;
    }

    esp_bd_addr_t addr;
    int values[6];

    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) != 6) {
        return false; // invalide
    }

    for (int i = 0; i < 6; i++) {
        if (values[i] < 0 || values[i] > 0xFF) {
            return false; // out of range
        }
        addr[i] = static_cast<uint8_t>(values[i] & 0xFF); // secure
    }

    // for some reason the last byte is always inc by 1 when set
    if (addr[5] != 0x00) {
        addr[5] -= 1; 
    }

    esp_err_t err = esp_base_mac_addr_set(addr);
    return err == ESP_OK;
}

void BluetoothService::clearBondedDevices() {
    int dev_num = esp_ble_get_bond_device_num();
    if (dev_num == 0) return;

    esp_ble_bond_dev_t* bonded = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    if (!bonded) return;

    if (esp_ble_get_bond_device_list(&dev_num, bonded) == ESP_OK) {
        for (int i = 0; i < dev_num; ++i) {
            esp_ble_remove_bond_device(bonded[i].bd_addr);
        }
    }

    free(bonded);
}

const uint8_t BluetoothService::HID_REPORT_MAP[] = {
    // Mouse report
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x85, 0x01,        //     Report ID (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x01,        //     Input (Cnst,Var,Abs)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data,Var,Rel)
    0xC0,              //   End Collection
    0xC0,              // End Collection

    // Keyboard report
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (224)
    0x29, 0xE7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Cnst,Var,Abs)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (1)
    0x29, 0x05,        //   Usage Maximum (5)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Cnst,Var,Abs)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data,Array)
    0xC0               // End Collection
};