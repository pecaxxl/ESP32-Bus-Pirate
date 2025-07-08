#include "SerialTerminalInput.h"
#include <Arduino.h>

char SerialTerminalInput::handler() {
    while (!Serial.available()) {}
    return Serial.read();
}

void SerialTerminalInput::waitPress() {
    while (!Serial.available()) {}
    Serial.read(); // discard
}

char SerialTerminalInput::readChar() {
    if (Serial.available()) {
        return Serial.read();
    }
    return KEY_NONE;
}