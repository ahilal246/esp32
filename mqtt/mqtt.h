// ============================================================
//  MQTT helper  -  you do not need to edit this file.
//
//    mqttBegin();                  call once in setup(), after wifiBegin()
//    mqttLoop();                   call always in loop()
//    mqttSend("topic", "value");   send a message, any topic
//    mqttConnected();              true while connected to the broker
//
//  And it calls YOUR function when a message arrives:
//    void onMqttMessage(String topic, String message) { ... }
//
//  Broker and topic list are in config.h
//  Needs the "PubSubClient" library (install from Library Manager).
// ============================================================
#pragma once

#include <Arduino.h>

// You write this function in your sketch.
void onMqttMessage(String topic, String message);

void mqttBegin();
void mqttLoop();

// Returns false if not connected right now.
// keep = true makes the broker remember the last value for new listeners.
bool mqttSend(const char* topic, String message, bool keep = false);

bool mqttConnected();
