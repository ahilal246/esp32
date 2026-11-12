// ============================================================
//  SETTINGS  -  this is the only ESP32 file you need to change
// ============================================================
#pragma once

// ---- 1. Your WiFi ------------------------------------------
// Must be the same WiFi your computer (the Python server) is on.
#define WIFI_NAME      "your-wifi-name"
#define WIFI_PASSWORD  "your-wifi-password"

// ---- 2. LED ------------------------------------------------
// On the ESP32-CAM (AI Thinker) board:
//    4  = the bright white flash LED next to the camera
//   33  = the small red LED on the back (set LED_ON_LEVEL to LOW)
// Or any free pin with your own LED.
#define LED_PIN        4
#define LED_ON_LEVEL   HIGH   // level that switches the LED on

// ---- 3. Camera ---------------------------------------------
#define CAMERA_SIZE     FRAMESIZE_VGA   // QVGA 320x240, VGA 640x480, SVGA 800x600
#define CAMERA_QUALITY  12              // 10 = sharp and big ... 30 = rough and small
