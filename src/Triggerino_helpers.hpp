/*********************************************************************
 *  Triggerino_helpers.hpp  –  support code
 *********************************************************************/
#pragma once

#include "utils/Types.h"
#include "hardware/DisplayManager.h"
#include "hardware/LEDManager.h"
#include "core/TriggerinoCore.h"
#include "network/NetworkManager.h"
/* ---------- standard & Adafruit libs --------------------------- */
#include <Arduino.h>

#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <Wire.h>

//#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
//#include "Adafruit_SI1145.h"
#include <vector>

using namespace Triggerino;

/* ===================  PINS ========================== */
#define SDA_2 33
#define SCL_2 32

/* ===================  GLOBALS  ================================ */

namespace Triggerino {
  extern DynamicJsonDocument configDoc;
  extern bool apMode;
  extern bool ethernetMode;
  extern WiFiUDP Udp;
  extern DNSServer dnsServer;
  extern WebServer server;
  extern QueueHandle_t      netQ;
  extern QueueHandle_t      httpQ;
  extern QueueHandle_t      animQ;
  extern EventGroupHandle_t evtGrp;
  extern const uint32_t     EVT_OLED_DIRTY;
  extern const uint32_t     EVT_UPDATE;
  bool isEthUp();
  bool isWiFiUp();
}

static Preferences prefs;

/* ===================  FILESYSTEM  ============================= */
static void initFilesystem() {
  if (!LittleFS.begin(true)) {
    Serial.println(" LittleFS mount failed"); ESP.restart();
  }
}

/* ===================  I²C + OLED  ============================= */
static void initI2CBus() {
  Wire.begin(SDA_2, SCL_2, 100000);
  Wire.setClock(400000);
}

/* ===================  CONFIG JSON  ============================ */
static bool loadConfigurationFromFS() {
  File f = LittleFS.open("/data/config.json", "r");
  if (!f) { Serial.println("⚠️ /data/config.json missing"); return false; }
  auto err = deserializeJson(configDoc, f); f.close();
  if (err) { Serial.println("JSON parse error"); return false; }
  return true;
}

/* ===================  WEB SERVER  ============================= */
static String mime(const String& f) {
  if (f.endsWith(".html")) return "text/html";
  if (f.endsWith(".css")) return "text/css";
  if (f.endsWith(".js")) return "application/javascript";
  if (f.endsWith(".json")) return "application/json";
  return "text/plain";
}

static void rootHandler() {
  if (apMode) {
    server.sendHeader("Location", "/wifi_setup");
    server.send(302);
    return;
  }
  File f = LittleFS.open("/data/index.html", "r");
  if (!f) { server.send(404, "text/plain", "index missing"); return; }
  server.streamFile(f, "text/html"); f.close();
}

static void statusHandler() {
  DynamicJsonDocument d(256);
  d["heap"] = ESP.getFreeHeap();
  d["eth"] = isEthUp();
  d["wifi"] = isWiFiUp();
  d["ap"] = apMode;
  String s; serializeJson(d, s);
  server.send(200, "application/json", s);
}

static void fileHandler() {
  String p = "/data" + server.uri();
  if (p.endsWith("/")) p += "index.html";
  if (!LittleFS.exists(p)) {
    server.send(404, "text/plain", "NF");
    return;
  }
  File f = LittleFS.open(p, "r");
  server.streamFile(f, mime(p));
  f.close();
}

void handleDeviceInfo();
void handleNetworkScan();

static void initWebServer() {
  server.on("/", HTTP_GET, rootHandler);
  server.on("/system_status", HTTP_GET, statusHandler);
  server.on("/device_info", HTTP_GET, handleDeviceInfo);
  server.on("/network_scan", HTTP_GET, handleNetworkScan);
  server.onNotFound(fileHandler);
  server.begin();
}

static void webServerLoop() {
  server.handleClient();
  if (apMode) dnsServer.processNextRequest();
}
