// MqttWifi - optional tiny helper to bring up WiFi in station mode without
// blocking. Skip this header if your project already manages its own network
// (WiFiManager, Ethernet, ...); MqttManager does not depend on it.

#pragma once

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "MqttWifi.h supports ESP32 and ESP8266 only"
#endif

namespace MqttWifi {

// Starts connecting and returns immediately. The WiFi stack keeps retrying
// on its own if the connection drops.
inline void begin(const char* ssid, const char* password, const char* hostname = nullptr) {
  WiFi.mode(WIFI_STA);
  if (hostname != nullptr) {
#if defined(ESP32)
    WiFi.setHostname(hostname);
#else
    WiFi.hostname(hostname);
#endif
  }
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
}

inline bool connected() { return WiFi.status() == WL_CONNECTED; }

}  // namespace MqttWifi
