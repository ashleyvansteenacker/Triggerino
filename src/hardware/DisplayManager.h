#pragma once
#include <Arduino.h>

namespace Triggerino {

void initDisplay();
void markDisplayDirty();
void redrawOLED();
void dispTask(void*);
}
