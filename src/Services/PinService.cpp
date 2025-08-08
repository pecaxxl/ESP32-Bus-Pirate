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
    return gpio_get_level((gpio_num_t)pin);
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

int PinService::readAnalog(uint8_t pin) {
    pinMode(pin, INPUT); 
    return analogRead(pin);
}

bool PinService::setupPwm(uint8_t pin, uint32_t freq, uint8_t dutyPercent) {
    if (dutyPercent > 100) dutyPercent = 100;

    int channel = pin % 16;
    int resolution = 8;

    if (!isPwmFeasible(freq, resolution))
        return false;

    bool ok = ledcSetup(channel, freq, resolution);
    if (!ok) return false;

    ledcAttachPin(pin, channel);
    uint32_t dutyVal = (dutyPercent * ((1 << resolution) - 1)) / 100;
    ledcWrite(channel, dutyVal);

    return true;
}

bool PinService::isPwmFeasible(uint32_t freq, uint8_t resolutionBits) {
    const uint32_t clkHz = 80000000; // horloge APB
    const uint32_t maxDiv = 1 << 20;   // 20 bit prescaler max
    if (resolutionBits > 20 || resolutionBits == 0) return false;

    uint32_t divParam = clkHz / (freq * (1 << resolutionBits));
    return divParam <= maxDiv && divParam > 0;
}