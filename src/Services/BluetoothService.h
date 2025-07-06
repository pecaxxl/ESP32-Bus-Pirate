#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "Data/AsciiHid.h"
// #include <esp_system.h>   // pour esp_read_mac
// #include <esp_mac.h>      // pour ESP_MAC_BT

enum class BluetoothMode {
    NONE,
    SERVER,
    CLIENT
};

struct ScannedDevice {
    std::string name;
    std::string address;
    int rssi;
};

class BluetoothService {
private:
    BLEHIDDevice* hid = nullptr;
    BLECharacteristic* mouseInput = nullptr;
    BLECharacteristic* keyboardInput = nullptr;
    bool connected = false;
    static const uint8_t HID_REPORT_MAP[];
    BluetoothMode mode = BluetoothMode::NONE;
public:
    // begin / end server BT
    void begin(const std::string& deviceName = "Bus-Pirate-Blueooth");
    void end();

    // Init client
    void init(const std::string& deviceName = "Bus-Pirate-Bluetooth");
    
    // Pair as client
    void pairWithAddress(const std::string& addrStr);      // pair <addr>

    // Connexion
    void onConnect();
    void onDisconnect();
    bool isConnected() const;

    // HID – Kb
    void sendKeyboardText(const std::string& text);
    void sendKeyboardReport(uint8_t modifier, const std::array<uint8_t, 6>& keys);

    // HID – Mouse
    void mouseMove(int16_t x, int16_t y);
    void clickMouse();  // Simule un clic 
    void sendMouseReport(int16_t x, int16_t y, uint8_t buttons);

    // Utils
    void sendEmptyReports();
    bool spoofMacAddress(const std::string& macStr);
    std::string getMacAddress();
    BluetoothMode getMode();
    void clearBondedDevices();

    std::vector<ScannedDevice> scanDevices(int seconds = 10);
    std::vector<std::string> connectTo(const std::string& addr);
};

