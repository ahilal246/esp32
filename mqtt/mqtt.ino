// ============================================================
//  Example sketch. Settings and topic list are in config.h
// ============================================================
#include "mqtt.h"

#define LED_PIN 2

unsigned long lastSend = 0;

// ---- RECEIVE: runs every time a message arrives --------------
// Add one "if" per topic. The topic must also be listed in config.h
void onMqttMessage(String topic, String message) {
  Serial.println(topic + " = " + message);

  if (topic == "ahilal/led") {
    digitalWrite(LED_PIN, message == "1" ? HIGH : LOW);
  }

  if (topic == "ahilal/relay") {
    // do something with message
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  mqttBegin();
}

void loop() {
  mqttLoop();

  // ---- SEND: use any topic you like, no list needed ----------
  if (millis() - lastSend >= 5000) {
    lastSend = millis();
    mqttSend("ahilal/uptime", String(millis() / 1000));
    // mqttSend("ahilal/temperature", String(temperature));
  }
}
