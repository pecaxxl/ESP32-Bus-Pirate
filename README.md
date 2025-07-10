# ESP32 Bus Pirate

**ESP32 Bus Pirate** is an open-source firmware that turns your device into a multi-protocol hacker's tool, inspired by the [legendary Bus Pirate](https://buspirate.com/).

It supports sniffing, sending, scripting, and interacting with various digital protocols (I2C, UART, 1-Wire, SPI, etc.) via a serial terminal or web-based CLI.

![Demo showing the different mode of the ESP32 Bus Pirate](images/demo.gif)

## Features

- Interactive command-line interface (CLI) via **USB Serial or WiFi Web**.
- **Modes for:**
  - HiZ (default)
  - I2C
  - SPI
  - UART / Half-Duplex UART
  - 1-WIRE
  - 2WIRE / 3WIRE (planned)
  - DIO (Digital I/O)
  - Infrared (device-b-gone, send and receive)
  - USB (HID, mouse, keyboard, gamepad, storage)
  - Bluetooth (BLE HID, scan, spoofing, sniffing)
  - Wi-Fi (scan, AP, connect, sniff, spoofing)
  - JTAG
  - LED control

- **Protocol sniffers** for I2C, Wi-Fi, Bluetooth, 1Wire.
- Baudrate **auto-detection** and various tools for UART.
- Scripting using **Bus Pirate-style bytecode** instructions.
- Direct I/O management, PWM, pin state.
- Massive infrared protocol support.
- Web interface with **live terminal**, or a classic **serial CLI**.

## Supported Devices

- **M5 Cardputer**

![An M5 Stack Cardputer device](images/cardputer.jpg)

- **M5 Stick C Plus 2**

![An M5 Stick C Plus 2 device](images/m5stick.jpg)

## Wiki

📚 Visit the **[Wiki](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki)** for detailed documentation on every mode and command.

Includes:
- [Mode overviews](wiki/Modes)
- [Instruction syntax](wiki/Instructions)
- [Serial/Web setup](wiki/Serial)
- [Python scripting examples](wiki/Python)


## Getting Started

1. 🔧 Flash the firmware  
   - Download the latest release from the [Releases](https://github.com/your-username/esp32-bus-pirate/releases) page, and flash it using your favorite tool (`esptool.py`, `PlatformIO`, etc.).
   - You can also burn it on [M5Burner](https://docs.m5stack.com/en/download), in the M5stick or Cardputer category.

2. 🔌 Connect via Serial or Web
   - Serial: any terminal app (see [Connect via Serial](https://github.com/your-username/esp32-bus-pirate/wiki/Connect-via-Serial))
   - Web: configure Wi-Fi and access the CLI via browser

3. 🧪 Use commands like:
   ```bash
   mode
   help
   scan
   sniff
   ...

## ESP32 Bus Pirate on M5 Devices
![ESP32 Bus Pirate running on M5 Stack devices](images/buspirate.jpg)
