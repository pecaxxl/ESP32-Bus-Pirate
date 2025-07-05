#include "SerialTerminalView.h"

void SerialTerminalView::initialize() {
    Serial.begin(baudrate); // Serial USB CDC, to the PC
    while (!Serial) {
        delay(10);
    }
}

void SerialTerminalView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {
    Serial.println("");
    Serial.println("");
    Serial.println("  ____                    _           _       ");
    Serial.println(" | __ ) _   _ ___   _ __ (_)_ __ __ _| |_ ___ ");
    Serial.println(" |  _ \\| | | / __| | '_ \\| | '__/ _` | __/ _ \\");
    Serial.println(" | |_) | |_| \\__ \\ | |_) | | | | (_| | ||  __/");
    Serial.println(" |____/ \\__,_|___/ | .__/|_|_|  \\__,_|\\__\\___|");
    Serial.println("                   |_|                        ");
    Serial.println("     Version 0.1           Ready to board");
    Serial.println("");
    Serial.println(" Type 'mode' to start or 'help' for commands");
    Serial.println("");

}

void SerialTerminalView::print(const std::string& text) {
    Serial.print(text.c_str());
}

void SerialTerminalView::println(const std::string& text) {
    Serial.println(text.c_str());
}

void SerialTerminalView::printPrompt(const std::string& mode) {
    if (!mode.empty()) {
        Serial.print(mode.c_str());
        Serial.print("> ");
    } else {
        Serial.print("> ");
    }
}

void SerialTerminalView::clear() {
    Serial.write(27);  // ESC
    Serial.print("[2J"); // erase screen
    Serial.write(27);
    Serial.print("[H");  // default cursor pos
}

void SerialTerminalView::setBaudrate(unsigned long baud) {
    baudrate = baud;
}