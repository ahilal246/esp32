// Shows how to keep a bigger project modular: every feature is its own class
// that receives the shared MqttManager and registers its own topics. The main
// sketch only wires things together, so features can be copied between
// projects without touching the MQTT code.

#include <WiFi.h>
#include <MqttManager.h>
#include <MqttWifi.h>

const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";
const char* MQTT_HOST = "192.168.1.10";

// --- Feature 1: a relay controlled over MQTT --------------------------------
class RelayModule {
 public:
  RelayModule(MqttManager& mqtt, uint8_t pin, const char* name) : _mqtt(mqtt), _pin(pin), _name(name) {}

  void begin() {
    pinMode(_pin, OUTPUT);
    String base = String("~/") + _name;  // e.g. garage/node1/relay1
    _stateTopic = base;
    _mqtt.subscribe((base + "/set").c_str(), [this](const String&, const String& payload) {
      set(payload.equalsIgnoreCase("ON"));
    });
  }

  void set(bool on) {
    _on = on;
    digitalWrite(_pin, on ? HIGH : LOW);
    publishState();
  }

  void publishState() { _mqtt.publish(_stateTopic.c_str(), _on ? "ON" : "OFF", true); }

 private:
  MqttManager& _mqtt;
  uint8_t _pin;
  String _name;
  String _stateTopic;
  bool _on = false;
};

// --- Feature 2: a periodic sensor reading -----------------------------------
class AnalogSensorModule {
 public:
  AnalogSensorModule(MqttManager& mqtt, uint8_t pin, const char* topic, uint32_t intervalMs)
      : _mqtt(mqtt), _pin(pin), _topic(topic), _intervalMs(intervalMs) {}

  void loop() {
    if (!_mqtt.connected() || millis() - _last < _intervalMs) {
      return;
    }
    _last = millis();
    _mqtt.publish(_topic.c_str(), String(analogRead(_pin)));
  }

 private:
  MqttManager& _mqtt;
  uint8_t _pin;
  String _topic;
  uint32_t _intervalMs;
  uint32_t _last = 0;
};

// --- Wiring -----------------------------------------------------------------
WiFiClient net;
MqttManager mqtt(net);
RelayModule relay1(mqtt, 26, "relay1");
RelayModule relay2(mqtt, 27, "relay2");
AnalogSensorModule light(mqtt, 34, "~/light", 5000);

void setup() {
  Serial.begin(115200);
  MqttWifi::begin(WIFI_SSID, WIFI_PASSWORD, "garage-node1");

  MqttConfig config;
  config.host = MQTT_HOST;
  config.baseTopic = "garage/node1";

  mqtt.setNetworkCheck(MqttWifi::connected);
  mqtt.onConnect([] {
    // Re-announce retained state after every (re)connect.
    relay1.publishState();
    relay2.publishState();
  });

  relay1.begin();
  relay2.begin();
  mqtt.begin(config);
}

void loop() {
  mqtt.loop();
  light.loop();
}
