# MqttManager – reusable MQTT module for ESP32

A small drop-in MQTT layer for ESP32 projects (Arduino framework, works in
PlatformIO and the Arduino IDE). It wraps
[PubSubClient](https://github.com/knolleary/pubsubclient) and takes care of
the parts every project needs:

- **Non-blocking auto-reconnect** with exponential backoff
- **Per-topic handlers** with MQTT wildcards (`+`, `#`)
- **Automatic re-subscribe** after every reconnect
- **Base topic shorthand** – `~/led` expands to `<baseTopic>/led`
- **Availability** – `online` / `offline` on `~/status` via Last Will
- **Transport independent** – pass any Arduino `Client` (WiFi, TLS, Ethernet)

## Folder layout

```
mqtt/
├── src/
│   ├── MqttManager.h / .cpp   the module
│   └── MqttWifi.h             optional WiFi helper (not required)
├── examples/
│   ├── BasicPubSub/           minimal publish + subscribe
│   └── ModularDevice/         several feature classes sharing one MqttManager
├── library.json               PlatformIO manifest
└── library.properties         Arduino IDE manifest
```

## Adding it to a project

**PlatformIO** – copy the `mqtt` folder into your project's `lib/` folder.
PubSubClient is installed automatically (declared in `library.json`).

**Arduino IDE** – copy the `mqtt` folder into `Documents/Arduino/libraries/`
and install *PubSubClient* from the Library Manager.

## Quick start

```cpp
#include <WiFi.h>
#include <MqttManager.h>
#include <MqttWifi.h>

WiFiClient net;
MqttManager mqtt(net);

void setup() {
  MqttWifi::begin("ssid", "password");

  MqttConfig config;
  config.host = "192.168.1.10";
  config.baseTopic = "home/demo";

  mqtt.setNetworkCheck(MqttWifi::connected);  // don't try while WiFi is down

  mqtt.subscribe("~/led/set", [](const String& topic, const String& payload) {
    digitalWrite(2, payload == "ON");
    mqtt.publish("~/led", payload, true);
  });

  mqtt.begin(config);
}

void loop() {
  mqtt.loop();  // must be called continuously
}
```

`subscribe()` can be called before or after `begin()`, and before the
connection exists – subscriptions are stored and applied on every connect.

## Configuration (`MqttConfig`)

| Field | Default | Meaning |
|---|---|---|
| `host` | – (required) | Broker hostname or IP |
| `port` | `1883` | Use `8883` for TLS |
| `clientId` | auto (`esp32-<mac>`) | Must be unique per device on the broker |
| `username` / `password` | none | Broker login |
| `baseTopic` | client ID | Prefix that replaces `~` in topics |
| `useAvailability` | `true` | Publish online/offline + register Last Will |
| `availabilityTopic` | `~/status` | |
| `payloadOnline` / `payloadOffline` | `online` / `offline` | |
| `keepAliveSec` | `30` | |
| `socketTimeoutSec` | `5` | Longest time a connect attempt can block |
| `bufferSize` | `512` | Max packet size (topic + payload); raise for big JSON |
| `cleanSession` | `true` | |
| `reconnectMinMs` / `reconnectMaxMs` | `1000` / `30000` | Backoff range |

## API

| Call | Purpose |
|---|---|
| `begin(config)` | Apply configuration and start connecting |
| `loop()` | Keep the connection alive; call from `loop()` |
| `subscribe(filter, handler, qos)` | Handler gets `(const String& topic, const String& payload)` |
| `subscribeRaw(filter, handler, qos)` | Handler gets `(const char* topic, const uint8_t* payload, size_t length)` – for binary data |
| `unsubscribe(filter)` | |
| `publish(topic, payload, retain)` | Payload as `const char*`, `String`, or bytes + length |
| `onConnect(fn)` / `onDisconnect(fn)` | Connection events (e.g. re-publish state) |
| `setNetworkCheck(fn)` | Skip connect attempts while `fn()` is false |
| `connected()` / `state()` | Status; `state()` is the PubSubClient error code |
| `disconnect()` / `reconnect()` | Stop / resume manually (e.g. before deep sleep) |
| `expandTopic("~/x")` | Returns the full topic |
| `client()` | The underlying `PubSubClient` |

## TLS

```cpp
#include <WiFiClientSecure.h>

WiFiClientSecure net;
MqttManager mqtt(net);

// in setup():
net.setCACert(ROOT_CA_PEM);
config.port = 8883;
```

## Good to know

- Publishing is **QoS 0** (PubSubClient limitation); subscriptions support
  QoS 0 and 1. `publish()` returns `false` while disconnected – messages are
  not queued.
- While the broker is unreachable, one connect attempt can block `loop()` for
  up to `socketTimeoutSec`; the backoff keeps those attempts rare.
- Keep handlers short. Do not call `delay()` inside them.
- For a multi-feature project, follow `examples/ModularDevice`: each feature
  takes a `MqttManager&` and registers its own topics.
