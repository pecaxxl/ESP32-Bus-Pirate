#pragma once
#include <string>
#include <vector>
#include <esp_system.h> 
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Managers/UserInputManager.h"
#include "Transformers/ArgTransformer.h"
#include "Services/SystemService.h"
#include "Services/WifiService.h"
#include "States/GlobalState.h"

class SysInfoShell {
public:
    SysInfoShell(ITerminalView& terminalView,
                 IInput& terminalInput,
                 UserInputManager& userInputManager,
                 ArgTransformer& argTransformer,
                 SystemService& systemService,
                 WifiService& wifiService);

    void run();

private:
    std::vector<std::string> actions = {
        " 📊 System summary",
        " 📟 Hardware info",
        " 🗄️  Memory",
        " 🧩 Partitions",
        " 🗂️  LittleFS",
        " 🧰 NVS stats",
        " 📒 NVS entries",
        " 🌐 Network",
        " 🔄 Reboot",
        "🚪 Exit"
    };

    const char* resetReasonToStr(int r) {
        switch (static_cast<esp_reset_reason_t>(r)) {
            case ESP_RST_POWERON:   return "Power-On";
            case ESP_RST_EXT:       return "External";
            case ESP_RST_SW:        return "Software";
            case ESP_RST_PANIC:     return "Panic";
            case ESP_RST_INT_WDT:   return "Interrupt WDT";
            case ESP_RST_TASK_WDT:  return "Task WDT";
            case ESP_RST_WDT:       return "Other WDT";
            case ESP_RST_DEEPSLEEP: return "Deep-Sleep Wake";
            case ESP_RST_BROWNOUT:  return "Brownout";
            case ESP_RST_SDIO:      return "SDIO";
            default:                return "Unknown";
        }
    }

    const char* flashModeToStr(int m) {
        switch (m) {
            case 0: return "QIO";
            case 1: return "QOUT";
            case 2: return "DIO";
            case 3: return "DOUT";
            case 4: return "FAST_READ";
            case 5: return "SLOW_READ";
            default:   return "?";
        }
    }

    const char* wifiModeToStr(int m) {
        switch (m) {
            case 0: return "NULL";
            case 1: return "STA";
            case 2: return "AP";
            case 3: return "AP+STA";
            default:return "?";
        }
    }

    const char* wlStatusToStr(int s) {
        switch (s) {
            case 0:   return "IDLE";
            case 1:   return "NO_SSID";
            case 2:   return "SCAN_DONE";
            case 3:   return "CONNECTED";
            case 4:   return "CONNECT_FAILED";
            case 5:   return "CONNECTION_LOST";
            case 6:   return "DISCONNECTED";
            case 7:   return "AP_LISTENING";
            case 8:   return "AP_CONNECTED";
            case 9:   return "AP_FAILED";
            case 10:  return "PROVISIONING";
            case 11:  return "PROVISIONING_FAILED";
            case 255: return "NO_SHIELD";
            default:  return "?";
        }
    }

    // Actions
    void cmdSummary();
    void cmdHardwareInfo();
    void cmdMemory();
    void cmdPartitions();
    void cmdFS();
    void cmdNVS(bool listEntries);
    void cmdNet();
    void cmdReboot(bool hard = false);

    ITerminalView&     terminalView;
    IInput&            terminalInput;
    UserInputManager&  userInputManager;
    ArgTransformer&    argTransformer;
    SystemService&     systemService;
    WifiService&       wifiService;
    GlobalState&       state = GlobalState::getInstance();
};
