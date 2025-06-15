#include "DisplayManager.h"
#include "utils/Types.h"
#include <ETH.h>
#include <WiFi.h>
#include "hardware/ButtonManager.h"
#include "network/NetworkManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "logo.h"


namespace Triggerino {

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool displayDirty = false;

// ---- TEMPORARY extern declarations until modules are migrated ----
extern bool isEthUp();
extern bool isWiFiUp();
extern bool apMode;
extern std::vector<Button> buttons;


void initDisplay() {
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Triggerino Booting..."));
display.setTextSize(2);
   display.setCursor(0, 50);
  display.print(F("V0.0.1 DEV"));
  display.display();
  delay(3000);  // Optional boot delay
  markDisplayDirty(); // Force redraw after logo
}


void markDisplayDirty() {
  displayDirty = true;
}

void redrawOLED() {
  if (!displayDirty || !display.width()) return;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Triggerino V0.0.1");

  if (isEthUp())        display.println("ETH : " + ETH.localIP().toString());
  else if (isWiFiUp())  display.println("WiFi: " + WiFi.localIP().toString());
  else if (apMode)      display.println("AP  : " + WiFi.softAPIP().toString());
  else                  display.println("NET : none");

  display.println();

  for (uint8_t i = 0; i < buttons.size() && i < 2; i++) {
    display.printf("%-12s %c\n", buttons[i].name.c_str(),
                                 buttons[i].stableState ? '*' : '-');
  }

  display.setCursor(0, 48); display.printf("RAM:%u", ESP.getFreeHeap());
  display.setCursor(0, 56); display.println("ashl.ee/triggerino");
  display.display();

  displayDirty = false;
}

void dispTask(void*) {
  for (;;) {
    redrawOLED();                       // Only draws if displayDirty is true
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


}
