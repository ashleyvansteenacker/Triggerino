#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>


namespace Triggerino {
  
// RTOS Objects
extern QueueHandle_t netQ;
extern QueueHandle_t httpQ;

/* ===================  ANIMATION SYSTEM  ======================== */
enum AnimationType {
  IDLE, BOOT, PULSE, CHASE, RAINBOW,
  BREATHE, STROBE, FIRE, WAVE,
  SOLID, TWINKLE, ERROR, SUCCESS,
  NETWORK_ACTIVITY
};

struct AnimationCommand {
  AnimationType type;
  uint8_t r, g, b, w;
  uint32_t duration;
};

struct AnimationState {
  AnimationType type;
  uint8_t r, g, b, w;
  uint32_t duration, startTime, frame;
  uint8_t phase;
  uint16_t param1, param2;
  bool active;
};

/* ===================  DATA STRUCTURES  ========================= */
struct Button {
  int pin; bool pullup; bool inverted; int debounce_ms;
  String name, description;
  bool lastReading, stableState;
  unsigned long lastDebounceTime;
};

struct Sensor {
  String type; int address; String name;
  unsigned long read_interval, lastRead;
  JsonObject params;
};

struct Output {
  String type, name, host, path; int port;
  JsonObject params;
};

struct Mapping {
  String input_name, input_type, output_name, condition;
  JsonObject data, trigger_params;
};

enum class JobType : uint8_t {
  ART,
  OSC,
  HTTP,
  TCP,
  HTTP_SLOW
};

struct NetJob {
  JobType type;
  Output* out;
  DynamicJsonDocument* payloadDoc;
  JsonObject payload;

  NetJob(JobType t, Output* o, DynamicJsonDocument* doc)
    : type(t), out(o), payloadDoc(doc), payload(doc->as<JsonObject>()) {}
};

// JSON configuration object (defined in Types.cpp)
extern DynamicJsonDocument configDoc;




} // namespace Triggerino
