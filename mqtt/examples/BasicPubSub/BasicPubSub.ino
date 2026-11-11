// Minimal example: connect, receive commands, publish a value every 10 s.
//
// Test with mosquitto:
//   mosquitto_sub -h <broker> -t 'home/demo/#' -v
//   mosquitto_pub -h <broker> -t 'home/demo/led/set' -m ON

#include <WiFi.h>
#include <MqttManager.h>
#include <MqttWifi.h>

const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";
const char* MQTT_HOST = "192.168.1.10";

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

WiFiClient net;
MqttManager mqtt(net);

uint32_t lastPublish = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  MqttWifi::begin(WIFI_SSID, WIFI_PASSWORD);

  MqttConfig config;
  config.host = MQTT_HOST;
  config.baseTopic = "home/demo";  // "~/" in topics expands to this
  // config.username = "user";
  // config.password = "secret";

  mqtt.setNetworkCheck(MqttWifi::connected);
  mqtt.onConnect([] { Serial.println("MQTT connected"); });
  mqtt.onDisconnect([] { Serial.println("MQTT disconnected"); });

  // home/demo/led/set  ->  ON / OFF
  mqtt.subscribe("~/led/set", [](const String& topic, const String& payload) {
    bool on = payload.equalsIgnoreCase("ON");
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
    mqtt.publish("~/led", on ? "ON" : "OFF", true);  // retained state
  });

  mqtt.begin(config);
}

void loop() {
  mqtt.loop();

  if (mqtt.connected() && millis() - lastPublish >= 10000) {
    lastPublish = millis();
    mqtt.publish("~/uptime", String(millis() / 1000));
  }
}
