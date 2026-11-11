#include "mqtt.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

static WiFiClient net;
static PubSubClient client(net);
static unsigned long lastTry = 0;

static void received(char* topic, byte* data, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)data[i];
  onMqttMessage(String(topic), message);
}

static void connectToBroker() {
  // Client ID must be different on every device, so build it from the chip ID.
  String id = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("MQTT connecting... ");
  bool ok = (strlen(MQTT_USER) > 0) ? client.connect(id.c_str(), MQTT_USER, MQTT_PASS)
                                    : client.connect(id.c_str());
  if (!ok) {
    Serial.println("failed, will retry in 5 s (error " + String(client.state()) + ")");
    return;
  }
  Serial.println("connected");

  // Listen to every topic from config.h
  for (unsigned int i = 0; i < sizeof(LISTEN_TOPICS) / sizeof(LISTEN_TOPICS[0]); i++) {
    client.subscribe(LISTEN_TOPICS[i]);
    Serial.println("  listening to: " + String(LISTEN_TOPICS[i]));
  }
}

void mqttBegin() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(received);
  connectToBroker();
  lastTry = millis();
}

void mqttLoop() {
  if (client.connected()) {
    client.loop();
    return;
  }
  // Connection lost: try again every 5 seconds, but only while WiFi is up.
  if (WiFi.status() == WL_CONNECTED && millis() - lastTry >= 5000) {
    lastTry = millis();
    connectToBroker();
  }
}

bool mqttSend(const char* topic, String message, bool keep) {
  return client.publish(topic, message.c_str(), keep);
}

bool mqttConnected() { return client.connected(); }
