#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include "Enums/InfraredProtocolEnum.h"
#include "Enums/ModeEnum.h"
#include "Enums/TerminalTypeEnum.h"

class GlobalState {
private:
    //Pin in use
    std::vector<uint8_t> protectedPins;

    // Builtin
    uint8_t ledPin = 21;

    // SPI
    uint8_t spiCSPin = 12;
    uint8_t spiCLKPin = 40;
    uint8_t spiMISOPin = 39;
    uint8_t spiMOSIPin = 14;
    uint32_t spiFrequency = 8000000;

    // WiFi AP Credentials
    std::string apName = "ESP32-Bus-Pirate";
    std::string apPassword = "readytoboard";

    // NVS
    std::string nvsNamespace = "wifi_settings";
    std::string nvsSsidField = "ssid";
    std::string nvsPasswordField = "pass";

    // Terminal Web UI
    std::string terminalIp = "0.0.0.0";

    // Terminal transmission mode
    TerminalTypeEnum terminalMode = TerminalTypeEnum::Serial;

    //  Current selected mode
    ModeEnum currentMode = ModeEnum::HIZ;

    // PC Terminal Serial Configuration
    unsigned long serialTerminalBaudRate = 115200;

    // OneWire Default Pin
    uint8_t oneWirePin = 1; // pin par défaut

    // UART Default Configuration
    unsigned long uartBaudRate = 9600;
    uint32_t uartConfig = 0x800001c; // SERIAL_8N1 by default
    bool uartInverted = false;
    uint8_t uartRxPin = 1;
    uint8_t uartTxPin = 2;
    uint8_t uartDataBits = 8;
    std::string uartParity = "None";
    bool uartFlowControl = false;
    uint8_t uartStopBits = 1;

    // HDUART Default Configuration
    unsigned long hdUartBaudRate = 9600;
    uint32_t hdUartConfig = 0x800001c; // SERIAL_8N1
    bool hdUartInverted = false;
    uint8_t hdUartPin = 1;
    uint8_t hdUartDataBits = 8;
    std::string hdUartParity = "N";
    bool hdUartFlowControl = false;
    uint8_t hdUartStopBits = 1;

    // I2C Default Configuration
    uint8_t i2cSclPin = 1;
    uint8_t i2cSdaPin = 2;
    uint32_t i2cFrequency = 100000;

    // Infrared Default Configuration
    uint8_t infraredPin = 44; 
    uint8_t infraredRxPin = 1;
    InfraredProtocolEnum infraredProtocol = InfraredProtocolEnum::_NEC;

    // SD Card File Limits
    size_t fileCountLimit = 512;
    size_t fileCacheLimit = 64;

public:
    GlobalState(const GlobalState&) = delete;
    GlobalState& operator=(const GlobalState&) = delete;

    static GlobalState& getInstance() {
        static GlobalState instance;
        return instance;
    }

    // Builtin
    uint8_t getLedPin() const { return ledPin; }

    // SPI
    uint8_t getSpiCSPin() const { return spiCSPin; }
    uint8_t getSpiCLKPin() const { return spiCLKPin; }
    uint8_t getSpiMISOPin() const { return spiMISOPin; }
    uint8_t getSpiMOSIPin() const { return spiMOSIPin; }
    uint32_t getSpiFrequency() const { return spiFrequency; }
    
    void setSpiCSPin(uint8_t pin) { spiCSPin = pin; }
    void setSpiCLKPin(uint8_t pin) { spiCLKPin = pin; }
    void setSpiMISOPin(uint8_t pin) { spiMISOPin = pin; }
    void setSpiMOSIPin(uint8_t pin) { spiMOSIPin = pin; }
    void setSpiFrequency(uint32_t freq) { spiFrequency = freq; }

    // AP WiFi
    const std::string& getApName() const { return apName; }
    const std::string& getApPassword() const { return apPassword; }

    void setApName(const std::string& name) { apName = name; }
    void setApPassword(const std::string& pass) { apPassword = pass; }

    // Terminal IP
    const std::string& getTerminalIp() const { return terminalIp; }
    void setTerminalIp(const std::string& ip) { terminalIp = ip; }

    // Terminal transmission mode
    TerminalTypeEnum getTerminalMode() const { return terminalMode; }
    void setTerminalMode(TerminalTypeEnum mode) { terminalMode = mode; }

    // Current Mode
    ModeEnum getCurrentMode() const { return currentMode; }
    void setCurrentMode(ModeEnum mode) { currentMode = mode; }

    // Serial Terminal Baud
    unsigned long getSerialTerminalBaudRate() const { return serialTerminalBaudRate; }
    void setSerialTerminalBaudRate(unsigned long rate) { serialTerminalBaudRate = rate; }

    // OneWire
    uint8_t getOneWirePin() const { return oneWirePin; }
    void setOneWirePin(uint8_t pin) { oneWirePin = pin; }

    // UART
    unsigned long getUartBaudRate() const { return uartBaudRate; }
    uint32_t getUartConfig() const { return uartConfig; }
    uint8_t getUartRxPin() const { return uartRxPin; }
    uint8_t getUartTxPin() const { return uartTxPin; }
    uint8_t getUartDataBits() const { return uartDataBits; }
    const std::string& getUartParity() const { return uartParity; }
    uint8_t getUartStopBits() const { return uartStopBits; }
    bool isUartInverted() const { return uartInverted; }
    bool isUartFlowControl() const { return uartFlowControl; }

    void setUartBaudRate(unsigned long b) { uartBaudRate = b; }
    void setUartConfig(uint32_t c) { uartConfig = c; }
    void setUartInverted(bool inv) { uartInverted = inv; }
    void setUartRxPin(uint8_t pin) { uartRxPin = pin; }
    void setUartTxPin(uint8_t pin) { uartTxPin = pin; }
    void setUartDataBits(uint8_t bits) { uartDataBits = bits; }
    void setUartParity(const std::string& parityVal) { uartParity = parityVal; }
    void setUartFlowControl(bool enabled) { uartFlowControl = enabled; }
    void setUartStopBits(uint8_t bits) { uartStopBits = bits; }

    // HDUART
    unsigned long getHdUartBaudRate() const { return hdUartBaudRate; }
    uint32_t getHdUartConfig() const { return hdUartConfig; }
    uint8_t getHdUartPin() const { return hdUartPin; }
    uint8_t getHdUartDataBits() const { return hdUartDataBits; }
    const std::string& getHdUartParity() const { return hdUartParity; }
    uint8_t getHdUartStopBits() const { return hdUartStopBits; }
    bool isHdUartInverted() const { return hdUartInverted; }
    bool isHdUartFlowControl() const { return hdUartFlowControl; }

    void setHdUartBaudRate(unsigned long b) { hdUartBaudRate = b; }
    void setHdUartConfig(uint32_t c) { hdUartConfig = c; }
    void setHdUartInverted(bool inv) { hdUartInverted = inv; }
    void setHdUartPin(uint8_t pin) { hdUartPin = pin; }
    void setHdUartDataBits(uint8_t bits) { hdUartDataBits = bits; }
    void setHdUartParity(const std::string& parityVal) { hdUartParity = parityVal; }
    void setHdUartFlowControl(bool enabled) { hdUartFlowControl = enabled; }
    void setHdUartStopBits(uint8_t bits) { hdUartStopBits = bits; }

    // I2C
    uint8_t getI2cSdaPin() const { return i2cSdaPin; }
    uint8_t getI2cSclPin() const { return i2cSclPin; }
    uint32_t getI2cFrequency() const { return i2cFrequency; }

    void setI2cSdaPin(uint8_t pin) { i2cSdaPin = pin; }
    void setI2cSclPin(uint8_t pin) { i2cSclPin = pin; }
    void setI2cFrequency(uint32_t freq) { i2cFrequency = freq; }

    // Infrared
    uint8_t getInfraredTxPin() const { return infraredPin; }
    uint8_t getInfraredRxPin() const { return infraredRxPin; }
    void setInfraredTxPin(uint8_t pin) { infraredPin = pin; }
    void setInfraredRxPin(uint8_t pin) { infraredRxPin = pin; }
    InfraredProtocolEnum getInfraredProtocol() const { return infraredProtocol ; }
    void setInfraredProtocol(InfraredProtocolEnum prot) {  infraredProtocol = prot; }

    // SD File Limits
    size_t getFileCountLimit() const { return fileCountLimit; }
    size_t getFileCacheLimit() const { return fileCacheLimit; }

    void setFileCountLimit(size_t count) { fileCountLimit = count; }
    void setFileCacheLimit(size_t cache) { fileCacheLimit = cache; }

    // NVS
    const std::string& getNvsNamespace() const { return nvsNamespace; }
    void setNvsNamespace(const std::string& ns) { nvsNamespace = ns; }

    const std::string& getNvsPasswordField() const { return nvsPasswordField; }
    void setNvsPasswordField(const std::string& f) { nvsPasswordField = f; }

    const std::string& getNvsSsidField() const { return nvsSsidField; }
    void setNvsSsidField(const std::string& f) { nvsSsidField = f; }

    // Protected
    const std::vector<uint8_t>& getProtectedPins() const {
        return protectedPins;
    }

    bool isPinProtected(uint8_t pin) const {
        return std::find(protectedPins.begin(), protectedPins.end(), pin) != protectedPins.end();
    }

    // Constructor
    GlobalState() {
        #ifdef LED_PIN
            ledPin = LED_PIN;
        #endif
        #ifdef SPI_CS_PIN
            spiCSPin = SPI_CS_PIN;
        #endif
        #ifdef SPI_CLK_PIN
            spiCLKPin = SPI_CLK_PIN;
        #endif
        #ifdef SPI_MISO_PIN
            spiMISOPin = SPI_MISO_PIN;
        #endif
        #ifdef SPI_MOSI_PIN
            spiMOSIPin = SPI_MOSI_PIN;
        #endif
        #ifdef ONEWIRE_PIN
            oneWirePin = ONEWIRE_PIN;
        #endif
        #ifdef UART_BAUD
            uartBaudRate = UART_BAUD;
        #endif
        #ifdef UART_RX_PIN
            uartRxPin = UART_RX_PIN;
        #endif
        #ifdef UART_TX_PIN
            uartTxPin = UART_TX_PIN;
        #endif
        #ifdef HDUART_BAUD
            hdUartBaudRate = HDUART_BAUD;
        #endif
        #ifdef HDUART_PIN
            hdUartPin = HDUART_PIN;
        #endif
        #ifdef I2C_SCL_PIN
            i2cSclPin = I2C_SCL_PIN;
        #endif
        #ifdef I2C_SDA_PIN
            i2cSdaPin = I2C_SDA_PIN;
        #endif
        #ifdef I2C_FREQ
            i2cFrequency = I2C_FREQ;
        #endif
        #ifdef IR_TX_PIN
            infraredPin = IR_TX_PIN;
        #endif
        #ifdef IR_RX_PIN
            infraredRxPin = IR_RX_PIN;
        #endif

        #ifdef PROTECTED_PINS
        {
            std::string pinsStr = PROTECTED_PINS;
            std::stringstream ss(pinsStr);
            std::string item;
            while (std::getline(ss, item, ',')) {
                protectedPins.push_back(static_cast<uint8_t>(std::stoi(item)));
            }
        }
        #endif
    }
};
