// src/network/NetworkManager.cpp
#include "NetworkManager.h"
#include "utils/Types.h"
#include "hardware/LEDManager.h"
#include "hardware/DisplayManager.h"
#include <WiFi.h>
#include <ETH.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

using namespace Triggerino;

namespace Triggerino {

bool apMode = false;
bool ethernetMode = true;
WiFiUDP Udp;

static Preferences prefs;
WebServer server(80);
DNSServer dnsServer; 

static const IPAddress apIP(192, 168, 4, 1);
static const IPAddress netMask(255, 255, 255, 0);
static const char* ap_ssid = "Triggerino-Setup";
static const char* ap_pwd  = "TriggerIt";

bool isEthUp() {
  return ETH.linkUp() && ETH.localIP() != IPAddress(0, 0, 0, 0);
}

bool isWiFiUp() {
  return WiFi.status() == WL_CONNECTED;
}

static bool connectSavedWiFi() {
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid", "");
  String pwd  = prefs.getString("password", "");
  prefs.end();
  if (!ssid.length()) return false;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pwd.c_str());
  for (uint8_t i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) delay(500);
  return WiFi.status() == WL_CONNECTED;
}

static void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMask);
  WiFi.softAP(ap_ssid, ap_pwd);
  dnsServer.start(53, "*", apIP);
  apMode = true;
  ethernetMode = false;
  queueAnimation(AnimationType::BREATHE, 255, 128, 0, 0, 0);
}

void initNetwork() {
  WiFi.mode(WIFI_OFF);
  ETH.begin(1, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  unsigned long t0 = millis();
  while (!isEthUp() && millis() - t0 < 10000) delay(500);

  if (isEthUp()) {
    ethernetMode = true;
    apMode = false;
    Serial.print("ETH IP: "); Serial.println(ETH.localIP());
    queueAnimation(AnimationType::SUCCESS, 0, 255, 0, 50, 2000);
  } else if (connectSavedWiFi()) {
    ethernetMode = false;
    apMode = false;
    Serial.print("WiFi IP: "); Serial.println(WiFi.localIP());
    queueAnimation(AnimationType::SUCCESS, 0, 0, 255, 50, 2000);
  } else {
    startAccessPoint();
    Serial.print("AP  IP: "); Serial.println(WiFi.softAPIP());
  }
}

void networkHousekeeping() {
  static unsigned long lastLinkChk = 0;
  if (millis() - lastLinkChk < 2000) return;
  lastLinkChk = millis();

  bool eth = isEthUp();
  bool wifi = isWiFiUp();

  if (ethernetMode && !eth) {
    ethernetMode = false;
    queueAnimation(AnimationType::ERROR, 255, 165, 0, 0, 1000);
    if (!connectSavedWiFi()) startAccessPoint();
    markDisplayDirty();
  }
  if (!ethernetMode && eth) {
    dnsServer.stop(); WiFi.softAPdisconnect(true); WiFi.mode(WIFI_OFF);
    ethernetMode = true;
    apMode = false;
    markDisplayDirty();
    queueAnimation(AnimationType::SUCCESS, 0, 255, 0, 50, 2000);
  }
}

void webServerLoopDNS() {
  if (apMode) dnsServer.processNextRequest();
}

} // namespace Triggerino
