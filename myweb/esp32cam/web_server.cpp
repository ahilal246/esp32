#include "web_server.h"

#include <Arduino.h>
#include <esp_http_server.h>
#include "camera.h"
#include "led.h"

#define CONTROL_PORT  80
#define STREAM_PORT   81

// The video is one endless answer made of JPEG pictures. This word
// marks where one picture ends and the next one starts.
#define BOUNDARY "frame"

// ---- LED -----------------------------------------------------

static esp_err_t sendLedState(httpd_req_t* request) {
  httpd_resp_set_type(request, "text/plain");
  return httpd_resp_sendstr(request, ledIsOn() ? "on" : "off");
}

static esp_err_t handleLedOn(httpd_req_t* request) {
  ledOn();
  Serial.println("LED on");
  return sendLedState(request);
}

static esp_err_t handleLedOff(httpd_req_t* request) {
  ledOff();
  Serial.println("LED off");
  return sendLedState(request);
}

// ---- Video ---------------------------------------------------

static esp_err_t handleStream(httpd_req_t* request) {
  httpd_resp_set_type(request, "multipart/x-mixed-replace;boundary=" BOUNDARY);
  Serial.println("Video: viewer connected");

  // Send picture after picture until the viewer disconnects.
  esp_err_t result = ESP_OK;
  while (result == ESP_OK) {
    camera_fb_t* picture = cameraTakePicture();
    if (picture == nullptr) {
      Serial.println("Video: could not take picture");
      result = ESP_FAIL;
      break;
    }

    char header[96];
    int headerLength = snprintf(header, sizeof(header),
        "--" BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
        (unsigned)picture->len);

    result = httpd_resp_send_chunk(request, header, headerLength);
    if (result == ESP_OK) result = httpd_resp_send_chunk(request, (const char*)picture->buf, picture->len);
    if (result == ESP_OK) result = httpd_resp_send_chunk(request, "\r\n", 2);

    cameraReturnPicture(picture);
  }

  Serial.println("Video: viewer disconnected");
  return result;
}

// ---- Start ---------------------------------------------------

static void addPage(httpd_handle_t server, const char* path, esp_err_t (*handler)(httpd_req_t*)) {
  httpd_uri_t page = {};
  page.uri     = path;
  page.method  = HTTP_GET;
  page.handler = handler;
  httpd_register_uri_handler(server, &page);
}

bool webServerBegin() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  // Server 1: LED commands
  httpd_handle_t controlServer = nullptr;
  config.server_port = CONTROL_PORT;
  if (httpd_start(&controlServer, &config) != ESP_OK) {
    Serial.println("Web server failed to start");
    return false;
  }
  addPage(controlServer, "/led/on",  handleLedOn);
  addPage(controlServer, "/led/off", handleLedOff);
  addPage(controlServer, "/led",     sendLedState);

  // Server 2: video
  httpd_handle_t streamServer = nullptr;
  config.server_port = STREAM_PORT;
  config.ctrl_port  += 1;   // every server needs its own internal port
  if (httpd_start(&streamServer, &config) != ESP_OK) {
    Serial.println("Video server failed to start");
    return false;
  }
  addPage(streamServer, "/stream", handleStream);

  Serial.println("Web server ready");
  return true;
}
