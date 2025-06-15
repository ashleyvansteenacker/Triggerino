#include "TriggerinoCore.h"
#include "utils/Types.h"
#include "hardware/ButtonManager.h"
#include "hardware/DisplayManager.h"

namespace Triggerino {

std::vector<Mapping> mappings;
std::vector<Output> outputs;
extern QueueHandle_t netQ;
extern QueueHandle_t httpQ;
extern DynamicJsonDocument configDoc;


// ========== Setup Outputs ==========
void setupOutputs() {
  outputs.clear();
  if (!configDoc.containsKey("outputs")) return;

  for (JsonObject o : configDoc["outputs"].as<JsonArray>()) {
    Output out;
    out.type  = o["type"].as<String>();
    out.name  = o["name"].as<String>();
    out.host  = o["host"].as<String>();
    out.port  = o["port"];
    out.path  = o["path"].as<String>();
    out.params = o["params"];
    outputs.push_back(out);
  }
}

// ========== Setup Mapping ==========
void setupMappings() {
  mappings.clear();

  if (!configDoc["mappings"].is<JsonArray>()) return;

  for (JsonObject o : configDoc["mappings"].as<JsonArray>()) {
    Mapping m;
    m.input_name = o["input_name"].as<String>();
    m.input_type = o["input_type"].as<String>();
    m.output_name = o["output_name"].as<String>();
    m.condition = o["condition"].as<String>();
    m.data = o["data"];
    m.trigger_params = o["trigger_params"];
    mappings.push_back(m);
  }
}
// ========== processButtonEvent ==========
void processButtonEvent(const String& name, const String& condition)
{
  Serial.printf("[DBG] processButtonEvent: name=%s, condition=%s\n", name.c_str(), condition.c_str());

  for (auto& m : mappings) {
    Serial.printf("  → Checking mapping: %s [%s] → %s if %s\n",
                  m.input_name.c_str(), m.input_type.c_str(),
                  m.output_name.c_str(), m.condition.c_str());

    if (m.input_name == name && m.input_type == "button" && m.condition == condition) {
      Serial.println("  → MATCH: Triggering output");
      triggerOutput(m.output_name, m.data);
    }
  }

  markDisplayDirty();
}


// ========== triggerOutput ==========
void triggerOutput(const String& outputName, JsonObject data)
{
  Serial.printf("[DBG] triggerOutput called for '%s'\n", outputName.c_str());

  for (auto& out : outputs) {
    Serial.printf("  - Checking against output '%s'\n", out.name.c_str());

    if (out.name == outputName) {
      Serial.printf("[Trigger] → %s (%s)\n", outputName.c_str(), out.type.c_str());

      JobType t = JobType::ART;
      if (out.type == "http") {
        t = (out.params.containsKey("slow") && out.params["slow"] == true)
              ? JobType::HTTP_SLOW : JobType::HTTP;
      } else if (out.type == "tcp") {
        t = JobType::TCP;
      } else if (out.type == "osc") {
        t = JobType::OSC;
      }

      DynamicJsonDocument* doc = new DynamicJsonDocument(512);
      doc->set(data);
      NetJob* job = new NetJob(t, &out, doc);

      if (t == JobType::HTTP_SLOW) {
        Serial.println("  → Queued to httpQ");
        xQueueSend(httpQ, &job, 0);
      } else {
        Serial.println("  → Queued to netQ");
        xQueueSend(netQ, &job, 0);
      }
      return;
    }
  }

  Serial.printf("[Warning] Output '%s' not found.\n", outputName.c_str());
}
}

