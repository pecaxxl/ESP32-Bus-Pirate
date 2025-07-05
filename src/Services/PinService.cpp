#include "PinService.h"

void PinService::setInput(uint8_t pin) {
    pinMode(pin, INPUT);
}

void PinService::setInputPullup(uint8_t pin) {
    pinMode(pin, INPUT_PULLUP);
}

void PinService::setOutput(uint8_t pin) {
    pinMode(pin, OUTPUT);
}

void PinService::setHigh(uint8_t pin) {
    setOutput(pin);  // force OUTPUT
    digitalWrite(pin, HIGH);
}

void PinService::setLow(uint8_t pin) {
    setOutput(pin);  // force OUTPUT
    digitalWrite(pin, LOW);
}

bool PinService::read(uint8_t pin) {
    return digitalRead(pin);
}

void PinService::togglePullup(uint8_t pin) {
    bool enabled = pullupState[pin]; // default false if not set

    if (enabled) {
        setInput(pin);
        pullupState[pin] = false;
    } else {
        setInputPullup(pin);
        pullupState[pin] = true;
    }
}
