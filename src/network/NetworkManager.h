#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

namespace Triggerino {

  extern bool apMode;
  extern bool ethernetMode;
  extern WiFiUDP Udp;

  void initNetwork();              // Initializes Ethernet, WiFi, or AP
  void networkHousekeeping();     // Handles failover logic (Ethernet <-> WiFi/AP)
  bool isEthUp();                 // True if Ethernet link is active
  bool isWiFiUp();                // True if WiFi connected

}
