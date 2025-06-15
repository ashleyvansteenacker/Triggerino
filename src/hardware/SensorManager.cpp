#include "hardware/SensorManager.h"
#include "utils/Types.h"
#include "core/TriggerinoCore.h"
#include <Wire.h>
/*----------include sensor libs here----------*/
#include <Adafruit_SI1145.h>

namespace Triggerino {

static std::vector<Sensor> sensors;
static Adafruit_SI1145 si1145;
static bool si1145_initialized = false;

void setupSensors() {
  sensors.clear();
  si1145_initialized = false;

  if (!configDoc.containsKey("sensors")) return;

  for (JsonObject o : configDoc["sensors"].as<JsonArray>()) {
    Sensor s;
    s.type = o["type"].as<String>();
    s.address = o["address"] | 0x00;
    s.name = o["name"].as<String>();
    s.read_interval = o["read_interval"] | 1000;
    s.params = o["params"];
    s.lastRead = 0;
    sensors.push_back(s);

    if (s.type == "SI1145" && !si1145_initialized) {
      if (!si1145.begin()) {
        Serial.println("Failed to initialize SI1145");
      } else {
        Serial.println("SI1145 initialized");
        si1145_initialized = true;
      }
    }
  }
}


static void processSensorEvent(const String& name, float v) {
  for (auto& m : mappings) {
    if (m.input_name == name && m.input_type == "sensor" && m.condition == "threshold") {
      Serial.println("=== processSensorEvent Debug ===");
      Serial.print("Sensor name: "); Serial.println(name);
      Serial.print("Sensor value: "); Serial.println(v, 4);
      Serial.print("Total mappings to check: "); Serial.println(mappings.size());

      bool fire = false;
      if (m.trigger_params.containsKey("temperature_threshold")) {
        float th = m.trigger_params["temperature_threshold"];
        String dir = m.trigger_params["threshold_direction"];
        fire = (dir == "above") ? (v > th) : (v < th);
      }

      if (fire) {
        triggerOutput(m.output_name, m.data);
      }
    }
  }
}

void readSensors() {
  for (auto& s : sensors) {
    if (millis() - s.lastRead < s.read_interval) continue;

    if (s.type == "generic_i2c") {
      Wire.beginTransmission(s.address);
      Wire.write(0);
      Wire.endTransmission();
      Wire.requestFrom(s.address, 2);
      if (Wire.available() < 2) continue;
      int val = (Wire.read() << 8) | Wire.read();
      processSensorEvent(s.name, val);
    }

   /* else if (s.type == "SI1145" && si1145_initialized) {
      String meas = s.params["measurement"] | "visible";
      float v = 0;
      if (meas == "visible")      v = si1145.readVisible();
      else if (meas == "ir")      v = si1145.readIR();
      else if (meas == "uv")      v = si1145.readUV() / 100.0f; // SI1145 gives UV index x100

      processSensorEvent(s.name, v);
    }*/

    s.lastRead = millis();
  }
}


} // namespace Triggerino
