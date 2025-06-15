// src/hardware/LEDManager.cpp
#include "Triggerino_helpers.hpp"
#include "utils/Types.h"
#include "LEDManager.h"
#include <cmath>

using namespace Triggerino;

Adafruit_NeoPixel Triggerino::statusLEDs(NUM_STATUS_LEDS, STATUS_LED_PIN, NEO_GRBW + NEO_KHZ800);

void Triggerino::initStatusLEDs() {
  statusLEDs.begin();
  statusLEDs.clear();
  statusLEDs.show();
  statusLEDs.setBrightness(50);
}

void Triggerino::queueAnimation(AnimationType type, uint8_t r, uint8_t g, uint8_t b, uint8_t w, uint32_t duration) {
  if (!animQ) return;
  AnimationCommand cmd{type, r, g, b, w, duration};
  xQueueSend(animQ, &cmd, 0);
}

static uint32_t wheelColor(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) return statusLEDs.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) {
    pos -= 85;
    return statusLEDs.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return statusLEDs.Color(pos * 3, 255 - pos * 3, 0);
}

void Triggerino::updateAnimation(AnimationState &state) {
  uint32_t elapsed = millis() - state.startTime;
  if (elapsed >= state.duration && state.type != AnimationType::IDLE) {
    state.active = false;
    statusLEDs.clear();
    statusLEDs.show();
    return;
  }

  float progress = (float)elapsed / state.duration;
  statusLEDs.clear();

  switch (state.type) {
    case AnimationType::BOOT: {
      int pos = (state.frame / 4) % NUM_STATUS_LEDS;
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        if (i == pos) {
          statusLEDs.setPixelColor(i, wheelColor(state.frame * 8));
        } else if (i == (pos + NUM_STATUS_LEDS - 1) % NUM_STATUS_LEDS) {
          uint32_t dimColor = wheelColor((state.frame - 4) * 8);
          statusLEDs.setPixelColor(i, (dimColor >> 2) & 0x3F3F3F3F);
        }
      }
      break;
    }
    case AnimationType::PULSE: {
      uint8_t brightness = (sin(progress * PI * 4) + 1) * 127;
      uint8_t r = (state.r * brightness) >> 8;
      uint8_t g = (state.g * brightness) >> 8;
      uint8_t b = (state.b * brightness) >> 8;
      uint8_t w = (state.w * brightness) >> 8;
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        statusLEDs.setPixelColor(i, statusLEDs.Color(r, g, b, w));
      }
      break;
    }
    case AnimationType::CHASE: {
      int pos = (int)(progress * NUM_STATUS_LEDS * 3) % NUM_STATUS_LEDS;
      statusLEDs.setPixelColor(pos, statusLEDs.Color(state.r, state.g, state.b, state.w));
      if (pos > 0) {
        statusLEDs.setPixelColor(pos - 1, statusLEDs.Color(state.r/4, state.g/4, state.b/4, state.w/4));
      }
      break;
    }
    case AnimationType::RAINBOW: {
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        uint8_t colorIndex = (state.frame * 4 + i * 64) & 255;
        statusLEDs.setPixelColor(i, wheelColor(colorIndex));
      }
      break;
    }
    case AnimationType::BREATHE: {
      float breath = (sin(progress * PI * 2) + 1) / 2;
      uint8_t brightness = breath * 255;
      uint8_t r = (state.r * brightness) >> 8;
      uint8_t g = (state.g * brightness) >> 8;
      uint8_t b = (state.b * brightness) >> 8;
      uint8_t w = (state.w * brightness) >> 8;
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        statusLEDs.setPixelColor(i, statusLEDs.Color(r, g, b, w));
      }
      break;
    }
    case AnimationType::STROBE: {
      bool on = (state.frame % 8) < 4;
      if (on) {
        for (int i = 0; i < NUM_STATUS_LEDS; i++) {
          statusLEDs.setPixelColor(i, statusLEDs.Color(state.r, state.g, state.b, state.w));
        }
      }
      break;
    }
    case AnimationType::FIRE: {
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        uint8_t heat = random(96, 255);
        uint8_t red = heat;
        uint8_t green = (heat > 128) ? (heat - 128) * 2 : 0;
        statusLEDs.setPixelColor(i, statusLEDs.Color(red, green, 0, 0));
      }
      break;
    }
    case AnimationType::WAVE: {
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        float wave = sin((progress * PI * 4) + (i * PI / 2));
        uint8_t brightness = ((wave + 1) / 2) * 255;
        uint8_t r = (state.r * brightness) >> 8;
        uint8_t g = (state.g * brightness) >> 8;
        uint8_t b = (state.b * brightness) >> 8;
        uint8_t w = (state.w * brightness) >> 8;
        statusLEDs.setPixelColor(i, statusLEDs.Color(r, g, b, w));
      }
      break;
    }
    case AnimationType::SOLID: {
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        statusLEDs.setPixelColor(i, statusLEDs.Color(state.r, state.g, state.b, state.w));
      }
      break;
    }
    case AnimationType::TWINKLE: {
      if (state.frame % 16 == 0) {
        int numTwinkles = random(1, NUM_STATUS_LEDS);
        for (int i = 0; i < numTwinkles; i++) {
          int pos = random(NUM_STATUS_LEDS);
          statusLEDs.setPixelColor(pos, statusLEDs.Color(state.r, state.g, state.b, state.w));
        }
      }
      break;
    }
    case AnimationType::ERROR: {
      bool on = (state.frame % 16) < 8;
      if (on) {
        for (int i = 0; i < NUM_STATUS_LEDS; i++) {
          statusLEDs.setPixelColor(i, statusLEDs.Color(255, 0, 0, 0));
        }
      }
      break;
    }
    case AnimationType::SUCCESS: {
      if (progress < 0.3) {
        for (int i = 0; i < NUM_STATUS_LEDS; i++) {
          statusLEDs.setPixelColor(i, statusLEDs.Color(0, 255, 0, 50));
        }
      } else {
        uint8_t fade = 255 * (1.0 - (progress - 0.3) / 0.7);
        for (int i = 0; i < NUM_STATUS_LEDS; i++) {
          statusLEDs.setPixelColor(i, statusLEDs.Color(0, fade, 0, fade / 5));
        }
      }
      break;
    }
    case AnimationType::NETWORK_ACTIVITY: {
      int center = NUM_STATUS_LEDS / 2;
      int radius = progress * (NUM_STATUS_LEDS / 2 + 1);
      for (int i = 0; i < NUM_STATUS_LEDS; i++) {
        int distance = abs(i - center);
        if (distance <= radius) {
          uint8_t brightness = 255 * (1.0 - (float)distance / radius);
          statusLEDs.setPixelColor(i, statusLEDs.Color(0, brightness, 255, brightness / 4));
        }
      }
      break;
    }
    case AnimationType::IDLE:
    default:
      break;
  }

  statusLEDs.show();
  state.frame++;
}

void Triggerino::updateStatusAnimation() {
  static uint32_t lastStatusUpdate = 0;
  static uint8_t statusFrame = 0;

  if (millis() - lastStatusUpdate < 100) return;
  lastStatusUpdate = millis();

  statusLEDs.clear();

  if (isEthUp()) {
    uint8_t pulse = (sin(statusFrame * 0.1f) + 1) * 10 + 20;
    statusLEDs.setPixelColor(0, statusLEDs.Color(0, 100, 0, pulse));
  } else if (isWiFiUp()) {
    uint8_t pulse = (sin(statusFrame * 0.1f) + 1) * 10 + 20;
    statusLEDs.setPixelColor(0, statusLEDs.Color(0, 0, 100, pulse));
  } else if (apMode) {
    uint8_t breath = (sin(statusFrame * 0.05f) + 1) * 64 + 64;
    statusLEDs.setPixelColor(0, statusLEDs.Color(breath, breath / 2, 0, 0));
  } else {
    uint8_t pulse = (sin(statusFrame * 0.08f) + 1) * 64 + 64;
    statusLEDs.setPixelColor(0, statusLEDs.Color(pulse, 0, 0, 0));
  }

  statusLEDs.show();
  statusFrame++;
}
