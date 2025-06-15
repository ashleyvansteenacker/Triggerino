#pragma once

#include "utils/Types.h"
#include <ArduinoJson.h>

namespace Triggerino {

void sendArtNet(const Output& out, JsonVariantConst payload);
void sendOSC(const Output& out, JsonVariantConst payload);
void sendHTTP(const Output& out, JsonVariantConst payload);
void sendTCP(const Output& out, JsonVariantConst payload);

// Optional helper
void queueNetJob(Output& out, JsonObject data);

}
