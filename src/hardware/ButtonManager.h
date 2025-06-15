#pragma once
#include <Arduino.h>
#include <vector>
#include "../utils/Types.h"

namespace Triggerino {

extern std::vector<Button> buttons;

void initButtons();
void readButtons();

}
