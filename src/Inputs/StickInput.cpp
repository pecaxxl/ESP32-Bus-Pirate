#include "StickInput.h"

StickInput::StickInput() {
    M5.begin();
}

char StickInput::mapButton() {
    M5.update();

    if (M5.BtnA.wasPressed()) return KEY_OK;
    if (M5.BtnB.wasPressed()) return KEY_RETURN_CUSTOM;

    return KEY_NONE;
}

char StickInput::readChar() {
    return mapButton();
}

char StickInput::handler() {
    char c = KEY_NONE;
    while ((c = mapButton()) == KEY_NONE) {
        delay(10);
    }
    return c;
}

void StickInput::waitPress() {
    while (mapButton() == KEY_NONE) {
        delay(10);
    }
}
