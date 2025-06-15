#include "ButtonManager.h"
#include "DisplayManager.h"
#include "utils/Types.h"
#include "core/TriggerinoCore.h"

namespace Triggerino {

extern DynamicJsonDocument configDoc;

std::vector<Button> buttons;

void initButtons() {
  buttons.clear();
  if (!configDoc.containsKey("buttons")) return;

  for (JsonObject o : configDoc["buttons"].as<JsonArray>()) {
    Button b;
    b.pin         = o["pin"] | -1;
    b.pullup      = o["pullup"] | true;
    b.inverted    = o["inverted"] | false;
    b.debounce_ms = o["debounce_ms"] | 50;
    b.name        = o["name"] | "Button";
    b.description = o["description"] | "";

    if (b.pin < 0) continue;

    pinMode(b.pin, b.pullup ? INPUT_PULLUP : INPUT);

    bool state = digitalRead(b.pin);
    if (b.inverted ^ b.pullup) state = !state;

    b.lastReading = b.stableState = state;
    b.lastDebounceTime = millis();

    buttons.push_back(b);
  }

  markDisplayDirty();
}

void readButtons() {
  for (auto& b : buttons) {
    bool state = digitalRead(b.pin);
    if (b.inverted ^ b.pullup) state = !state;

    if (state != b.lastReading) {
      b.lastDebounceTime = millis();
      b.lastReading = state;
    }

    if ((millis() - b.lastDebounceTime) >= b.debounce_ms && state != b.stableState) {
      b.stableState = state;
      Triggerino::processButtonEvent(b.name, state ? "pressed" : "released");
    }
  }
}

}
