/*
  ESP32-CAM side of the WebRTC + Cloudflare Tunnel project
  Board: AI Thinker ESP32-CAM

  What this sketch does
    - Connects to your WiFi and prints the board's IP address.
    - Port 81:  /stream    live video (MJPEG). server.js reads this.
    - Port 80:  /button1   toggles the flash LED (GPIO 4)
                /button2   toggles the small red LED on the back (GPIO 33)
                /          a simple test page with the video and both buttons

  These addresses match the defaults in server.js, so there you only have
  to fill in ESP32_IP.

  Files in this folder
    config.h              YOUR SETTINGS: WiFi, button pins, picture size
    esp32side.ino         this file: starts the parts below
    wifi_setup.h / .cpp   connects to WiFi
    camera.h / .cpp       starts the camera
    buttons.h / .cpp      the two outputs that the buttons switch
    web_server.h / .cpp   the test page, the buttons and the video
    web_page.h            the HTML of the test page

  Arduino IDE settings
    Tools > Board      > ESP32 Arduino > AI Thinker ESP32-CAM
    Tools > Port       > your USB-serial adapter
    Serial Monitor at 115200 baud to read the IP address
*/

#include <WiFi.h>
#include "config.h"
#include "wifi_setup.h"
#include "camera.h"
#include "buttons.h"
#include "web_server.h"

void setup() {
  Serial.begin(115200);
  Serial.println();

  buttonsBegin();   // outputs start in the OFF state

  if (!startCamera()) {
    delay(5000);
    ESP.restart();
  }

  connectWiFi();
  startServers();

  String ip = WiFi.localIP().toString();
  Serial.println("Ready.");
  Serial.println("  Board IP (put this in ESP32_IP in server.js): " + ip);
  Serial.println("  Test page: http://" + ip + "/");
  Serial.println("  Video:     http://" + ip + ":81/stream");
  Serial.println("  Buttons:   http://" + ip + "/button1   and   /button2");
}

void loop() {
  // The web servers run in the background. WiFi reconnects by itself.
  delay(1000);
}
