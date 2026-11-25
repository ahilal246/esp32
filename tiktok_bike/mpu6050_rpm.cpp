#include "mpu6050_rpm.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "config.h"

// MPU6050 register numbers (from the MPU6050 register map)
#define MPU_REG_SMPLRT_DIV    0x19
#define MPU_REG_CONFIG        0x1A
#define MPU_REG_GYRO_CONFIG   0x1B
#define MPU_REG_GYRO_XOUT_H   0x43   // 6 bytes: X high, X low, Y high, Y low, Z high, Z low
#define MPU_REG_PWR_MGMT_1    0x6B
#define MPU_REG_WHO_AM_I      0x75

#define GYRO_2000         0x18   // GYRO_CONFIG value for the biggest range, +-2000 deg/s
#define UNITS_PER_DEG_S   16.4f  // sensor units per deg/s in that range
#define READ_EVERY_MS     10     // 100 readings per second

static bool  working = false;
static float zeroPoint[3] = {0, 0, 0};   // what the gyro says when the wheel is still
static float rpm = 0;
static unsigned long lastRead  = 0;
static unsigned long lastCheck = 0;

static bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

static bool readRegisters(uint8_t reg, uint8_t* data, size_t count) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);   // no stop: the read below follows straight away
  if (Wire.requestFrom((uint8_t)MPU_ADDRESS, count, true) != count) return false;
  for (size_t i = 0; i < count; i++) data[i] = Wire.read();
  return true;
}

// Reads the 3 gyro axes in raw sensor units. False if the sensor didn't answer.
static bool readGyro(int16_t g[3]) {
  uint8_t d[6];
  if (!readRegisters(MPU_REG_GYRO_XOUT_H, d, 6)) return false;
  for (int i = 0; i < 3; i++) g[i] = (int16_t)((d[2 * i] << 8) | d[2 * i + 1]);
  return true;
}

// Wakes the sensor and sets it up for a spinning wheel. Also used to
// recover when a loose wire made the sensor reset itself.
static bool setupSensor() {
  uint8_t id;
  if (!readRegisters(MPU_REG_WHO_AM_I, &id, 1)) return false;   // nobody answered

  writeRegister(MPU_REG_PWR_MGMT_1, 0x80);   // reset to factory settings
  delay(100);
  bool ok = writeRegister(MPU_REG_PWR_MGMT_1, 0x01)          // wake up, use the gyro's clock
         && writeRegister(MPU_REG_CONFIG, 0x03)              // filter out vibration above ~40 Hz
         && writeRegister(MPU_REG_SMPLRT_DIV, 9)             // 1000 / (1 + 9) = 100 readings per second
         && writeRegister(MPU_REG_GYRO_CONFIG, GYRO_2000);
  delay(100);   // let the gyro settle
  return ok;
}

// The gyro never says exactly 0 when still (up to 20 deg/s off). Measure
// what it says so it can be subtracted. Tries again if the wheel moved.
static void measureZeroPoint() {
  Serial.println("MPU6050: measuring zero point, keep the wheel still...");
  for (int attempt = 0; attempt < 5; attempt++) {
    float   sum[3]  = {0, 0, 0};
    int16_t low[3]  = {32767, 32767, 32767};
    int16_t high[3] = {-32768, -32768, -32768};
    int count = 0;

    for (int i = 0; i < 100; i++) {   // 100 readings = 1 second
      int16_t g[3];
      if (readGyro(g)) {
        for (int a = 0; a < 3; a++) {
          sum[a] += g[a];
          if (g[a] < low[a])  low[a]  = g[a];
          if (g[a] > high[a]) high[a] = g[a];
        }
        count++;
      }
      delay(READ_EVERY_MS);
    }

    // Still = readings didn't change (more than 5 deg/s) AND are close to 0.
    // A wheel turning at a steady speed also gives steady readings, so the
    // second test is needed: real zero error is never above about 40 deg/s.
    bool still = count >= 50;
    float average[3];
    for (int a = 0; a < 3; a++) {
      average[a] = sum[a] / (count > 0 ? count : 1);
      if (high[a] - low[a] > 5 * UNITS_PER_DEG_S) still = false;
    }
    float size = sqrtf(average[0] * average[0] + average[1] * average[1] + average[2] * average[2]);
    if (size > 40 * UNITS_PER_DEG_S) still = false;

    if (still) {
      for (int a = 0; a < 3; a++) zeroPoint[a] = average[a];
      Serial.println("MPU6050: ready");
      return;
    }
    Serial.println("MPU6050: wheel moved, measuring again...");
  }
  Serial.println("MPU6050: wheel never stood still, RPM may read a few too high");
}

bool mpuBegin() {
  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN, 100000);
  working = setupSensor();
  if (!working) {
    Serial.println("MPU6050: not found! Check SDA, SCL, 3.3V and GND");
    return false;
  }
  Serial.println("MPU6050: found");
  measureZeroPoint();
  return true;
}

void mpuLoop() {
  unsigned long now = millis();
  if (now - lastRead < READ_EVERY_MS) return;
  float seconds = (now - lastRead) / 1000.0f;
  lastRead = now;

  int16_t g[3];
  if (working && readGyro(g)) {
    float x = (g[0] - zeroPoint[0]) / UNITS_PER_DEG_S;
    float y = (g[1] - zeroPoint[1]) / UNITS_PER_DEG_S;
    float z = (g[2] - zeroPoint[2]) / UNITS_PER_DEG_S;
    float degPerSecond = sqrtf(x * x + y * y + z * z);   // turning speed, any direction
    float rpmNow = degPerSecond / 6.0f;                   // 360 deg/s = 1 turn per second = 60 RPM

    // Smoothing: move part of the way towards the new value each time
    float part = seconds / (RPM_SMOOTHING_SECONDS + seconds);
    rpm += part * (rpmNow - rpm);
  }

  // Once a second, check the sensor is still there and still set up.
  // A loose wire can make it disappear or reset itself. If it is lost,
  // try to set it up again every second until it answers.
  if (now - lastCheck >= 1000) {
    lastCheck = now;
    uint8_t range = 0;
    bool fine = readRegisters(MPU_REG_GYRO_CONFIG, &range, 1) && range == GYRO_2000;
    if (!fine || !working) {
      if (working) Serial.println("MPU6050: lost! Check the wires");
      working = setupSensor();
      if (working) Serial.println("MPU6050: back");
    }
  }

  if (!working) rpm = 0;   // no sensor = counts as stopped (slow videos)
}

float wheelRpm() { return rpm; }

bool mpuWorking() { return working; }
