// ============================================================
//  MQTT helper  -  you do not need to edit this file.
//
//  It gives you three things:
//    mqttBegin();                  call once in setup()
//    mqttLoop();                   call always in loop()
//    mqttSend("topic", "value");   send a message, any topic
//
//  And it calls YOUR function when a message arrives:
//    void onMqttMessage(String topic, String message) { ... }
//
//  Needs the "PubSubClient" library (install from Library Manager).
// ============================================================
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// You write this function in your sketch.
void onMqttMessage(String topic, String message);

static WiFiClient   _mqttWifi;
static PubSubClient _mqtt(_mqttWifi);
static unsigned long _mqttLastTry = 0;

static void _mqttReceived(char* topic, byte* data, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)data[i];
  onMqttMessage(String(topic), message);
}

static void _mqttConnect() {
  // Client ID must be different on every device, so build it from the chip ID.
  String id = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("MQTT connecting... ");
  bool ok = (strlen(MQTT_USER) > 0) ? _mqtt.connect(id.c_str(), MQTT_USER, MQTT_PASS)
                                    : _mqtt.connect(id.c_str());
  if (!ok) {
    Serial.println("failed, will retry in 5 s (error " + String(_mqtt.state()) + ")");
    return;
  }
  Serial.println("connected");

  // Listen to every topic from config.h
  for (unsigned int i = 0; i < sizeof(LISTEN_TOPICS) / sizeof(LISTEN_TOPICS[0]); i++) {
    _mqtt.subscribe(LISTEN_TOPICS[i]);
    Serial.println("  listening to: " + String(LISTEN_TOPICS[i]));
  }
}

// Call once in setup(). Waits until WiFi is connected.
static void mqttBegin() {
  Serial.print("WiFi connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  _mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  _mqtt.setCallback(_mqttReceived);
  _mqttConnect();
  _mqttLastTry = millis();
}

// Call always in loop(). Reconnects by itself if the connection drops.
static void mqttLoop() {
  if (_mqtt.connected()) {
    _mqtt.loop();
    return;
  }
  if (WiFi.status() == WL_CONNECTED && millis() - _mqttLastTry >= 5000) {
    _mqttLastTry = millis();
    _mqttConnect();
  }
}

// Send a message. Returns false if not connected right now.
// keep = true makes the broker remember the last value for new listeners.
static bool mqttSend(const char* topic, String message, bool keep = false) {
  return _mqtt.publish(topic, message.c_str(), keep);
}

static bool mqttConnected() { return _mqtt.connected(); }
