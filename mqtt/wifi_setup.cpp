#include "wifi_setup.h"

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

void wifiBegin() {
  Serial.print("WiFi connecting");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);  // comes back by itself if the router drops
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}

bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }
