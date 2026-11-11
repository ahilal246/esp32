// MqttManager - reusable, non-blocking MQTT module for ESP32 (Arduino framework).
//
// Wraps PubSubClient and adds what every project ends up re-writing:
//   - automatic reconnect with exponential backoff (never blocks loop())
//   - per-topic handlers with MQTT wildcard support (+ and #)
//   - automatic re-subscribe after every reconnect
//   - base topic shorthand: "~/x" expands to "<baseTopic>/x"
//   - availability (Last Will): "online" / "offline" published for you
//
// The transport is injected (any Arduino `Client`), so the same code works
// with WiFiClient, WiFiClientSecure (TLS) or an Ethernet client.

#pragma once

#include <Arduino.h>
#include <Client.h>
#include <PubSubClient.h>
#include <functional>
#include <vector>

struct MqttConfig {
  // --- Broker -------------------------------------------------------------
  const char* host = nullptr;      // broker hostname or IP (required)
  uint16_t port = 1883;            // 1883 plain, 8883 TLS
  const char* clientId = nullptr;  // nullptr = auto-generated from chip ID
  const char* username = nullptr;  // nullptr = no authentication
  const char* password = nullptr;

  // --- Topics -------------------------------------------------------------
  // Topics starting with "~/" are expanded with this prefix.
  // nullptr = use the client ID as base topic.
  const char* baseTopic = nullptr;

  // --- Availability (Last Will and Testament) -----------------------------
  bool useAvailability = true;
  const char* availabilityTopic = "~/status";
  const char* payloadOnline = "online";
  const char* payloadOffline = "offline";

  // --- Tuning -------------------------------------------------------------
  uint16_t keepAliveSec = 30;
  uint16_t socketTimeoutSec = 5;  // max time a connect attempt may block
  uint16_t bufferSize = 512;      // max packet size (topic + payload + header)
  bool cleanSession = true;
  uint32_t reconnectMinMs = 1000;   // first retry delay
  uint32_t reconnectMaxMs = 30000;  // retry delay cap (doubles on each failure)
};

class MqttManager {
 public:
  using MessageHandler = std::function<void(const String& topic, const String& payload)>;
  using RawHandler = std::function<void(const char* topic, const uint8_t* payload, size_t length)>;
  using EventHandler = std::function<void()>;
  using NetworkCheck = std::function<bool()>;

  explicit MqttManager(Client& netClient);

  // Copies everything out of `config`; the struct may be a temporary.
  // Returns false if the configuration is unusable (no host).
  bool begin(const MqttConfig& config);

  // Call from loop() as often as possible. Never blocks while connected;
  // a reconnect attempt can take up to socketTimeoutSec.
  void loop();

  // Cleanly disconnect (publishes the offline payload first) and stop
  // reconnecting until begin() or reconnect() is called.
  void disconnect();
  void reconnect();

  bool connected();
  // PubSubClient state code of the last attempt (0 = connected, <0 network
  // error, >0 broker refused). See PubSubClient.h MQTT_* constants.
  int state();

  // Optional: skip connection attempts while this returns false,
  // e.g. [] { return WiFi.status() == WL_CONNECTED; }
  void setNetworkCheck(NetworkCheck check) { _networkCheck = check; }

  void onConnect(EventHandler handler) { _onConnect = handler; }
  void onDisconnect(EventHandler handler) { _onDisconnect = handler; }

  // --- Subscribing ----------------------------------------------------------
  // May be called at any time, also before the connection exists: the
  // subscription is stored and (re)applied on every connect.
  // `filter` may contain wildcards and the "~/" prefix. qos is 0 or 1.
  void subscribe(const char* filter, MessageHandler handler, uint8_t qos = 0);
  // Zero-copy variant for binary / large payloads. The payload pointer is
  // only valid until you publish from inside the handler.
  void subscribeRaw(const char* filter, RawHandler handler, uint8_t qos = 0);
  void unsubscribe(const char* filter);

  // --- Publishing (QoS 0) ---------------------------------------------------
  // Return false when not connected or the packet exceeds bufferSize.
  bool publish(const char* topic, const char* payload, bool retain = false);
  bool publish(const char* topic, const String& payload, bool retain = false);
  bool publish(const char* topic, const uint8_t* payload, size_t length, bool retain = false);

  // Expand "~/x" to "<baseTopic>/x"; other topics are returned unchanged.
  String expandTopic(const char* topic) const;
  const String& baseTopic() const { return _baseTopic; }
  const String& clientId() const { return _clientId; }

  // True if `topic` matches the MQTT `filter` (supports + and #).
  static bool topicMatches(const char* filter, const char* topic);

  // Escape hatch for anything not wrapped here.
  PubSubClient& client() { return _client; }

 private:
  struct Subscription {
    String pattern;  // as given by the caller, may start with "~/"
    String filter;   // expanded; refreshed in begin() once the base topic is known
    uint8_t qos;
    MessageHandler handler;
    RawHandler rawHandler;
  };

  void addSubscription(const char* filter, uint8_t qos, MessageHandler handler, RawHandler raw);
  bool attemptConnect();
  void handleMessage(char* topic, uint8_t* payload, unsigned int length);
  static String defaultClientId();

  PubSubClient _client;
  std::vector<Subscription> _subscriptions;

  String _host, _clientId, _username, _password, _baseTopic;
  String _availabilityTopic, _payloadOnline, _payloadOffline;
  uint16_t _port = 1883;
  bool _useAuth = false;
  bool _useAvailability = true;
  bool _cleanSession = true;
  uint32_t _reconnectMinMs = 1000;
  uint32_t _reconnectMaxMs = 30000;

  bool _enabled = false;
  bool _wasConnected = false;
  uint32_t _retryDelayMs = 0;  // 0 = try immediately
  uint32_t _lastAttemptMs = 0;

  NetworkCheck _networkCheck;
  EventHandler _onConnect;
  EventHandler _onDisconnect;
};
