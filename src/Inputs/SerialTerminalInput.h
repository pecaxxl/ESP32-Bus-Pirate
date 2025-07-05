#pragma once

#include <Inputs/IInput.h>
#include <Arduino.h>
#include <vector>

class SerialTerminalInput : public IInput {
public:
    char handler() override;
    void waitPress() override;
    char readChar() override;
};
