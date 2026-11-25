// ============================================================
//  SETTINGS  -  this is the only file you need to change
// ============================================================
#pragma once

// ---- 1. Speed rule (the "x" from the drawing) ---------------
// Wheel faster than RPM_X  ->  next TikTok video every FAST_SECONDS
// Wheel slower than RPM_X  ->  next TikTok video every SLOW_SECONDS
// RPM = wheel turns per minute. 120 is about 15 km/h on a normal
// adult bike wheel. README.md has a table for other speeds.
#define RPM_X          120
#define FAST_SECONDS   40
#define SLOW_SECONDS   200

// ---- 2. Bluetooth -------------------------------------------
#define BLE_NAME       "TikTok Remote"   // pick this in the phone's Bluetooth settings
#define BLE_MAKER      "DIY"
#define SCROLL_STEPS   1                 // raise to 2-3 if one scroll doesn't change the video

// ---- 3. MPU6050 wiring --------------------------------------
#define MPU_SDA_PIN    21
#define MPU_SCL_PIN    22
#define MPU_ADDRESS    0x68              // 0x69 if the AD0 pin is wired to 3.3V

// ---- 4. RPM smoothing ---------------------------------------
// The RPM is averaged over about this many seconds, so one quick
// push or bump doesn't change the speed mode.
// Bigger = steadier but slower to react.
#define RPM_SMOOTHING_SECONDS  2.0
