// ============================================================
//  SETTINGS  -  this is the only file you need to change
// ============================================================
#pragma once

// ---- 1. Your WiFi ------------------------------------------
#define WIFI_NAME      "your-wifi-name"
#define WIFI_PASSWORD  "your-wifi-password"

// ---- 2. Broker (same values as in your phone app) ----------
#define MQTT_BROKER    "broker.emqx.io"
#define MQTT_PORT      1883
#define MQTT_USER      ""     // leave "" if the broker has no login
#define MQTT_PASS      ""

// ---- 3. Topics the ESP32 LISTENS to ------------------------
// Add as many lines as you need. One topic per line, comma at the end.
// broker.emqx.io is public, so start every topic with something
// unique to you (here "ahilal/") so nobody else's device clashes.
static const char* const LISTEN_TOPICS[] = {
  "ahilal/led",
  "ahilal/relay",
  // "ahilal/another/topic",
};

// ---- 4. Camera (only for the ESP32-CAM AI Thinker board) ---
#define CAMERA_TOPIC        "ahilal/camera"   // put this topic in the app's Image panel
#define CAMERA_INTERVAL_MS  500               // time between pictures: 500 = 2 per second
#define CAMERA_SIZE         FRAMESIZE_QVGA    // QQVGA 160x120, QVGA 320x240, VGA 640x480
#define CAMERA_QUALITY      12                // 10 = sharp and big ... 30 = rough and small
// If the app shows no picture, change "" to "data:image/jpeg;base64,"
#define CAMERA_PREFIX       ""
