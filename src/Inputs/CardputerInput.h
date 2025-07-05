#pragma once

#include <map>
#include <M5Cardputer.h>
#include "IInput.h"
#include "InputKeys.h"


class CardputerInput : public IInput {
public:
    char handler() override;
    void waitPress() override;
    char readChar() override;
};
