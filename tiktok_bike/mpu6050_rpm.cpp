#include "mpu6050_rpm.h"

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include "config.h"

static MPU6050 mpu(Wire);
static bool  found = false;
static float rpm = 0;
static unsigned long lastRead = 0;

bool mpuBegin() {
  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN);

  // 3 = gyro range +-2000 deg/s, measures up to 333 RPM.
  // The default (+-500 deg/s) stops at 83 RPM: too slow for a bike wheel.
  found = (mpu.begin(3) == 0);
  if (!found) {
    Serial.println("MPU6050: not found! Check SDA, SCL, 3.3V and GND");
    return false;
  }

  Serial.println("MPU6050: calibrating, keep the wheel still...");
  delay(1000);
  mpu.calcOffsets(true, false);   // gyro only, the accelerometer isn't used
  Serial.println("MPU6050: ready");
  return true;
}

void mpuLoop() {
  if (!found || millis() - lastRead < 10) return;   // 100 readings a second
  lastRead = millis();

  mpu.update();
  float gx = mpu.getGyroX();   // deg/s
  float gy = mpu.getGyroY();
  float gz = mpu.getGyroZ();

  float degPerSecond = sqrt(gx * gx + gy * gy + gz * gz);   // works in any orientation
  float rpmNow = degPerSecond / 6;                          // 360 deg/s = 1 turn a second = 60 RPM

  rpm = 0.99 * rpm + 0.01 * rpmNow;   // smoothing: about the last 1 second
}

float wheelRpm() { return rpm; }
