#include "Triggerino_helpers.hpp"
#include <ArduinoJson.h>
#include <WiFi.h>


namespace Triggerino {
  extern WebServer server;
  extern DNSServer dnsServer;
}


void handleDeviceInfo() {
  DynamicJsonDocument doc(512);
  doc["chip_model"] = ESP.getChipModel();
  doc["chip_revision"] = ESP.getChipRevision();
  doc["chip_cores"] = ESP.getChipCores();
  doc["flash_size"] = ESP.getFlashChipSize();
  doc["sdk_version"] = ESP.getSdkVersion();
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleNetworkScan() {
  DynamicJsonDocument doc(2048);
  JsonArray networks = doc.createNestedArray("networks");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Encrypted";
  }
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
