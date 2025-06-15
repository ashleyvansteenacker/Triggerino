#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "utils/Types.h"

namespace Triggerino {

constexpr uint8_t NUM_STATUS_LEDS = 4;
constexpr uint8_t STATUS_LED_PIN = 2;

extern Adafruit_NeoPixel statusLEDs;

void initStatusLEDs();
void updateStatusAnimation();
void queueAnimation(AnimationType type, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0, uint32_t duration = 1000);
void updateAnimation(AnimationState &state);

} // namespace Triggerino
