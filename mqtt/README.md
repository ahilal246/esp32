# Simple MQTT for ESP32

Files:

| File | What it is |
|---|---|
| `config.h` | **Your settings**: WiFi, broker, list of topics to listen to |
| `mqtt.ino` | Example sketch: what to do when a message arrives, what to send |
| `wifi_setup.h` / `wifi_setup.cpp` | The WiFi code. No need to edit |
| `mqtt.h` / `mqtt.cpp` | The MQTT code. No need to edit |

## Setup

1. Arduino IDE → Library Manager → install **PubSubClient**.
2. Open `mqtt.ino`, fill in WiFi name and password in `config.h`, upload.
3. Open Serial Monitor at 115200 to see the connection and messages.

## Where to put topics

**Receive a topic** – two places:

1. `config.h` – add a line to `LISTEN_TOPICS`:
   ```cpp
   "ahilal/fan",
   ```
2. `mqtt.ino` – add an `if` inside `onMqttMessage`:
   ```cpp
   if (topic == "ahilal/fan") {
     // message holds the text that was sent
   }
   ```

**Send a topic** – one line, anywhere in your code, no list needed:

```cpp
mqttSend("ahilal/temperature", String(25.4));
```

There is no limit on the number of topics either way.

## Phone app

Use the same broker (`broker.emqx.io`, port `1883`, TCP). A button that
publishes `1` / `0` to `ahilal/led` switches the LED; a text widget
subscribed to `ahilal/uptime` shows the value the ESP32 sends.

`broker.emqx.io` is public: anyone can read or write your topics. Change
`ahilal/` to your own prefix, and don't use it for anything private.

## Using it in another project

Copy `config.h`, `wifi_setup.h`, `wifi_setup.cpp`, `mqtt.h` and `mqtt.cpp`
next to your sketch. Then:

```cpp
#include "wifi_setup.h"
#include "mqtt.h"

void onMqttMessage(String topic, String message) { /* your code */ }

void setup() { wifiBegin(); mqttBegin(); }
void loop()  { mqttLoop(); }
```

If a project already connects to WiFi its own way, leave out the two
`wifi_setup` files and just call `mqttBegin()` after WiFi is connected.
