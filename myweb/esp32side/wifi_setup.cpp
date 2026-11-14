#include "wifi_setup.h"

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

void connectWiFi() {
  WiFi.mode(WIFI_STA);
#ifdef USE_FIXED_IP
  WiFi.config(FIXED_IP, FIXED_GATEWAY, FIXED_SUBNET, FIXED_DNS);
#endif
  WiFi.setSleep(false);   // keeps the video smooth
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++tries > 60) {   // 30 seconds without success: start again
      Serial.println("\nNo WiFi. Check the name and password. Restarting...");
      ESP.restart();
    }
  }
  Serial.println();
}
