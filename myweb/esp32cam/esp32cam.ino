// ============================================================
//  ESP32-CAM: live video + LED switch over WiFi.
//  Settings are in config.h
//
//  Board in Arduino IDE: Tools -> Board -> AI Thinker ESP32-CAM
// ============================================================
#include "wifi_setup.h"
#include "camera.h"
#include "led.h"
#include "web_server.h"

void setup() {
  Serial.begin(115200);
  ledBegin();
  wifiBegin();        // prints the IP address the Python server needs
  cameraBegin();
  webServerBegin();
}

void loop() {
  // Nothing to do here: the web server runs in the background.
  delay(1000);
}
