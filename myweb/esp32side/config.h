// ============================================================
//  SETTINGS  -  this is the only file you need to change
// ============================================================
#pragma once

#include <Arduino.h>
#include "esp_camera.h"

// ---- 1. Your WiFi ------------------------------------------
const char* const WIFI_SSID     = "your-wifi-name";
const char* const WIFI_PASSWORD = "your-wifi-password";

// ---- 2. Buttons --------------------------------------------
// What each button controls. Change the pins to drive your own hardware
// (relay, motor driver input, buzzer...). Free pins when no SD card is
// used: 12, 13, 14, 15, 2.
const int  BUTTON1_PIN = 4;      // bright flash LED on the front
const bool BUTTON1_ON  = HIGH;   // this LED lights when the pin is HIGH

const int  BUTTON2_PIN = 33;     // small red LED on the back
const bool BUTTON2_ON  = LOW;    // this LED lights when the pin is LOW

// ---- 3. Camera ---------------------------------------------
// Picture size. Smaller is smoother. Options from small to large:
// FRAMESIZE_QVGA (320x240), FRAMESIZE_CIF (400x296), FRAMESIZE_VGA (640x480),
// FRAMESIZE_SVGA (800x600)
const framesize_t FRAME_SIZE = FRAMESIZE_QVGA;

// JPEG quality: 10 (best, most data) to 63 (worst). 12 is a good start.
const int JPEG_QUALITY = 20;

// ---- 4. Fixed address (optional) ---------------------------
// Give the board a fixed address so ESP32_IP in webside/config.js never changes.
// Remove the two slashes on the next line and set your own numbers.
// #define USE_FIXED_IP
#ifdef USE_FIXED_IP
const IPAddress FIXED_IP(192, 168, 1, 50);
const IPAddress FIXED_GATEWAY(192, 168, 1, 1);
const IPAddress FIXED_SUBNET(255, 255, 255, 0);
const IPAddress FIXED_DNS(8, 8, 8, 8);
#endif
