#pragma once
#include <ArduinoJson.h>
#include <vector>

namespace Triggerino {
  struct Mapping;
  struct Output;

  extern std::vector<Mapping> mappings;
  extern std::vector<Output> outputs;
  void setupOutputs();
  void setupMappings();
  void triggerOutput(const String& outputName, JsonObject data);
  void processButtonEvent(const String& name, const String& condition);
}
