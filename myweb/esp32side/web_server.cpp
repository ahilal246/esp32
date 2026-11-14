#include "web_server.h"

#include <Arduino.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "buttons.h"
#include "web_page.h"

static httpd_handle_t controlServer = NULL;  // port 80: page and buttons
static httpd_handle_t streamServer  = NULL;  // port 81: video

// ------------------------------------------------------- page and buttons ---

static esp_err_t sendText(httpd_req_t *req, const char *text) {
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, text, strlen(text));
}

static esp_err_t indexHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t button1Handler(httpd_req_t *req) {
  return sendText(req, toggleButton1() ? "Button 1: ON" : "Button 1: OFF");
}

static esp_err_t button2Handler(httpd_req_t *req) {
  return sendText(req, toggleButton2() ? "Button 2: ON" : "Button 2: OFF");
}

// ------------------------------------------------------------------ video ---

// Live video: one JPEG picture after another in a single long reply (MJPEG).
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY     = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t streamHandler(httpd_req_t *req) {
  char partHeader[64];
  esp_err_t res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  Serial.println("Stream started");
  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();   // take one picture
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      break;
    }
    size_t headerLen = snprintf(partHeader, sizeof(partHeader), STREAM_PART, (unsigned int)fb->len);
    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, partHeader, headerLen);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);                // give the memory back
    if (res != ESP_OK) break;                // the viewer disconnected
  }
  Serial.println("Stream stopped");
  return res;
}

// ------------------------------------------------------------------ start ---

// Connect one address (like "/button1") to the function that answers it.
static void addRoute(httpd_handle_t server, const char *path, esp_err_t (*handler)(httpd_req_t *)) {
  httpd_uri_t route = {};
  route.uri      = path;
  route.method   = HTTP_GET;
  route.handler  = handler;
  route.user_ctx = NULL;
  httpd_register_uri_handler(server, &route);
}

void startServers() {
  // Port 80: test page and buttons
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  if (httpd_start(&controlServer, &config) == ESP_OK) {
    addRoute(controlServer, "/", indexHandler);
    addRoute(controlServer, "/button1", button1Handler);
    addRoute(controlServer, "/button2", button2Handler);
  } else {
    Serial.println("Could not start the button server");
  }

  // Port 81: video. It runs as a separate server so the buttons keep
  // working while the video is streaming.
  httpd_config_t streamConfig = HTTPD_DEFAULT_CONFIG();
  streamConfig.server_port = 81;
  streamConfig.ctrl_port   = config.ctrl_port + 1;

  if (httpd_start(&streamServer, &streamConfig) == ESP_OK) {
    addRoute(streamServer, "/stream", streamHandler);
  } else {
    Serial.println("Could not start the video server");
  }
}
