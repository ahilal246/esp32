#include "camera.h"

#include <Arduino.h>
#include "config.h"

// Wiring of the camera on the AI Thinker ESP32-CAM board (fixed, do not change)
#define PIN_PWDN   32
#define PIN_RESET  -1
#define PIN_XCLK    0
#define PIN_SIOD   26
#define PIN_SIOC   27
#define PIN_Y9     35
#define PIN_Y8     34
#define PIN_Y7     39
#define PIN_Y6     36
#define PIN_Y5     21
#define PIN_Y4     19
#define PIN_Y3     18
#define PIN_Y2      5
#define PIN_VSYNC  25
#define PIN_HREF   23
#define PIN_PCLK   22

static bool cameraReady = false;

bool cameraBegin() {
  camera_config_t cam = {};
  cam.ledc_channel = LEDC_CHANNEL_0;
  cam.ledc_timer   = LEDC_TIMER_0;
  cam.pin_d0 = PIN_Y2;
  cam.pin_d1 = PIN_Y3;
  cam.pin_d2 = PIN_Y4;
  cam.pin_d3 = PIN_Y5;
  cam.pin_d4 = PIN_Y6;
  cam.pin_d5 = PIN_Y7;
  cam.pin_d6 = PIN_Y8;
  cam.pin_d7 = PIN_Y9;
  cam.pin_xclk     = PIN_XCLK;
  cam.pin_pclk     = PIN_PCLK;
  cam.pin_vsync    = PIN_VSYNC;
  cam.pin_href     = PIN_HREF;
  cam.pin_sccb_sda = PIN_SIOD;
  cam.pin_sccb_scl = PIN_SIOC;
  cam.pin_pwdn     = PIN_PWDN;
  cam.pin_reset    = PIN_RESET;
  cam.xclk_freq_hz = 20000000;
  cam.pixel_format = PIXFORMAT_JPEG;   // the camera makes the JPEG itself
  cam.frame_size   = CAMERA_SIZE;
  cam.jpeg_quality = CAMERA_QUALITY;

  if (psramFound()) {
    // Extra memory on the board: keep 2 pictures and always hand out the newest.
    cam.fb_location = CAMERA_FB_IN_PSRAM;
    cam.fb_count    = 2;
    cam.grab_mode   = CAMERA_GRAB_LATEST;
  } else {
    cam.fb_location = CAMERA_FB_IN_DRAM;
    cam.fb_count    = 1;
    cam.grab_mode   = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t error = esp_camera_init(&cam);
  if (error != ESP_OK) {
    Serial.println("Camera failed to start (error 0x" + String((uint32_t)error, HEX) + ")");
    cameraReady = false;
    return false;
  }

  Serial.println("Camera ready");
  cameraReady = true;
  return true;
}

camera_fb_t* cameraTakePicture() {
  if (!cameraReady) return nullptr;
  return esp_camera_fb_get();
}

void cameraReturnPicture(camera_fb_t* picture) {
  if (picture != nullptr) esp_camera_fb_return(picture);
}
