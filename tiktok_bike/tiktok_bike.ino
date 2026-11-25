// ============================================================
//  TikTok Bike: the faster you ride, the faster the videos change.
//
//  An MPU6050 on the bike wheel measures the wheel's RPM.
//    RPM above RPM_X  ->  next video every FAST_SECONDS (40 s)
//    RPM below RPM_X  ->  next video every SLOW_SECONDS (200 s)
//  The ESP32 acts as a Bluetooth mouse and scrolls down to change video.
//  No buttons needed.
//
//  Settings are in config.h. Wiring and mounting are in README.md
//  iPhone: turn on Settings > Accessibility > Touch > AssistiveTouch first.
// ============================================================
#include "config.h"
#include "tiktok_mouse.h"
#include "mpu6050_rpm.h"

unsigned long lastVideoChange = 0;   // when we last went to a new video
unsigned long lastPrint = 0;
bool wasPaired = false;

void setup() {
  Serial.begin(115200);
  mouseBegin();
  mpuBegin();   // keep the wheel still for a few seconds after switching on
  Serial.println("Pair with \"" BLE_NAME "\" in your phone's Bluetooth settings");
}

void loop() {
  mpuLoop();

  float rpm = wheelRpm();
  bool fast = rpm > RPM_X;
  unsigned long videoTime = (fast ? FAST_SECONDS : SLOW_SECONDS) * 1000UL;

  // Phone just connected: move the pointer aside, time the first video from now
  bool paired = mousePaired();
  if (paired && !wasPaired) {
    Serial.println("Phone connected");
    mouseParkCursor();
    lastVideoChange = millis();
  }
  if (!paired && wasPaired) Serial.println("Phone disconnected, waiting for it...");
  wasPaired = paired;

  // Has this video had its time? Then go to the next one.
  // The time counts from the last video change and follows your
  // speed right now: 40 s while fast, 200 s while slow.
  if (paired && millis() - lastVideoChange >= videoTime) {
    mouseNextVideo();
    lastVideoChange = millis();
    Serial.printf(">> Next video (RPM %.0f)\n", rpm);
  }

  // Status once a second. Open Serial Monitor at 115200 to see it.
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    Serial.printf("RPM %3.0f  %s  ", rpm, fast ? "FAST" : "slow");
    if (!mpuWorking()) Serial.print("(MPU6050 not answering)  ");
    if (paired) {
      unsigned long waited = millis() - lastVideoChange;
      unsigned long left = waited < videoTime ? (videoTime - waited + 999) / 1000 : 0;
      Serial.printf("next video in %lu s\n", left);
    } else {
      Serial.println("phone not connected");
    }
  }
}
