#include "mqtt.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <mbedtls/base64.h>
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

bool mqttSendBase64(const char* topic, const uint8_t* data, size_t length, const char* prefix) {
  if (!client.connected()) return false;

  // Base64 turns every 3 bytes into 4 letters, so we know the final size
  // up front. MQTT needs that size before the first byte is sent.
  size_t prefixLength = strlen(prefix);
  size_t textLength   = ((length + 2) / 3) * 4;

  if (!client.beginPublish(topic, prefixLength + textLength, false)) return false;

  bool ok = true;
  if (prefixLength > 0) ok = client.write((const uint8_t*)prefix, prefixLength) == prefixLength;

  // Convert and send 900 bytes at a time (900 bytes -> 1200 letters).
  // 900 is a multiple of 3, so the pieces join up into one valid Base64 text.
  const size_t PIECE = 900;
  unsigned char text[1201];
  for (size_t done = 0; ok && done < length; done += PIECE) {
    size_t count = (length - done < PIECE) ? (length - done) : PIECE;
    size_t made  = 0;
    ok = mbedtls_base64_encode(text, sizeof(text), &made, data + done, count) == 0 &&
         client.write(text, made) == made;
  }

  if (!client.endPublish()) ok = false;

  if (!ok) {
    // The message was cut off halfway, so the connection is in a broken
    // state. Drop it; mqttLoop() connects again by itself.
    Serial.println("MQTT: big message failed, reconnecting");
    client.disconnect();
  }
  return ok;
}
