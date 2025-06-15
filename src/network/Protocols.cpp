#include "Protocols.h"
#include "utils/Types.h"
#include "hardware/LEDManager.h"   // For queueAnimation()
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <ArduinoJson.h>
#include "utils/Types.h"  // or wherever you put the externs

namespace Triggerino {


// Global UDP instance (should be defined in NetworkManager.cpp or main)
extern WiFiUDP Udp;

void sendArtNet(const Output& out, JsonVariantConst payload) {
  JsonObjectConst data = payload.as<JsonObjectConst>();
  uint8_t pkt[530] = {0};
  strcpy((char*)pkt, "Art-Net");
  pkt[8] = 0x00; pkt[9] = 0x50; // OpCode: ArtDMX
  pkt[10] = 0; pkt[11] = 14;   // Protocol Version
  int uni = data["universe"] | 0;
  pkt[14] = uni & 0xFF;
  pkt[15] = (uni >> 8) & 0xFF;
  pkt[16] = 0x02; // Sequence
  pkt[17] = 0x00; // Physical
  if (data.containsKey("channels")) {
    JsonArrayConst ch = data["channels"];
    for (uint16_t i = 0; i < ch.size() && i < 512; i++)
      pkt[18 + i] = ch[i];
  }
  Udp.beginPacket(out.host.c_str(), out.port);
  Udp.write(pkt, 530);
  Udp.endPacket();
  queueAnimation(AnimationType::PULSE, 0, 255, 255, 50, 2000);
}

void sendOSC(const Output& out, JsonVariantConst payload) {
  const char* address = out.path.c_str();
  OSCMessage msg(address);

  if (payload.is<JsonObjectConst>()) {
    JsonObjectConst obj = payload.as<JsonObjectConst>();
    if (obj.containsKey("args") && obj["args"].is<JsonArrayConst>()) {
      JsonArrayConst args = obj["args"].as<JsonArrayConst>();
      for (JsonVariantConst arg : args) {
        if (arg.is<int>())        msg.add(arg.as<int>());
        else if (arg.is<float>()) msg.add(arg.as<float>());
        else if (arg.is<const char*>()) msg.add(arg.as<const char*>());
      }
    }
  } else if (payload.is<int>())        msg.add(payload.as<int>());
  else if (payload.is<float>())        msg.add(payload.as<float>());
  else if (payload.is<const char*>()) msg.add(payload.as<const char*>());

  Udp.beginPacket(out.host.c_str(), out.port);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  queueAnimation(AnimationType::PULSE, 255, 0, 0, 100, 2000);
  Serial.printf("[OSC] Sent to %s:%d (%s)\n", out.host.c_str(), out.port, address);
}

void sendHTTP(const Output& out, JsonVariantConst payload) {
  HTTPClient cli;
  String url = "http://" + out.host + ":" + String(out.port) + out.path;

  if (!cli.begin(url)) {
    Serial.printf("[HTTP] Failed to begin: %s\n", url.c_str());
    return;
  }

  cli.setTimeout(1000);
  cli.setReuse(false);
  cli.addHeader("Content-Type", "application/json");

  if (out.params.containsKey("auth")) {
    cli.addHeader("Authorization", "Bearer " + out.params["auth"].as<String>());
  }

  String method = "POST";
  if (out.params.containsKey("method")) {
    method = out.params["method"].as<String>();
    method.toUpperCase();
  }

  int httpCode = -1;
  String response;
  if (method == "POST") {
    String body;
    serializeJson(payload, body);
    httpCode = cli.POST(body);
  } else if (method == "GET") {
    httpCode = cli.GET();
  } else {
    Serial.printf("[HTTP] Unsupported method: %s\n", method.c_str());
    cli.end();
    queueAnimation(AnimationType::ERROR, 255, 0, 0, 0, 2000);
    return;
  }

  if (httpCode > 0) {
    response = cli.getString();
    Serial.printf("[HTTP] %s => %d\nResponse: %s\n", url.c_str(), httpCode, response.c_str());
    queueAnimation(AnimationType::PULSE, 255, 0, 255, 50, 2000);
  } else {
    Serial.printf("[HTTP] Request failed to %s\nCode: %d, Error: %s\n",
                  url.c_str(), httpCode, cli.errorToString(httpCode).c_str());
    queueAnimation(AnimationType::ERROR, 255, 0, 0, 0, 2000);
  }
  cli.end();
}

void sendTCP(const Output& out, JsonVariantConst payload) {
  WiFiClient c;
  if (!c.connect(out.host.c_str(), out.port)) return;
  String s;
  serializeJson(payload, s);
  c.print(s);
  c.stop();
}

// Queue NetJob to netQ from config-mapped triggers


void queueNetJob(Output &out, JsonObject data) {
  if (!netQ) return;

  using namespace Triggerino;

  JobType t = JobType::ART;
  if (out.type == "http") {
    t = out.params["slow"].is<bool>() && out.params["slow"] ? JobType::HTTP_SLOW : JobType::HTTP;
  } else if (out.type == "tcp") {
    t = JobType::TCP;
  } else if (out.type == "osc") {
    t = JobType::OSC;
  }

  DynamicJsonDocument* doc = new DynamicJsonDocument(512);
  doc->set(data);
  NetJob* job = new NetJob(t, &out, doc);

  QueueHandle_t q = (t == JobType::HTTP_SLOW) ? httpQ : netQ;
  if (!xQueueSend(q, &job, 0)) {
    delete doc;
    delete job;
  }
}


} // namespace Triggerino
