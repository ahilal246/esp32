#include "MqttManager.h"

MqttManager::MqttManager(Client& netClient) : _client(netClient) {}

bool MqttManager::begin(const MqttConfig& config) {
  if (config.host == nullptr || config.host[0] == '\0') {
    return false;
  }

  _host = config.host;
  _port = config.port;
  _clientId = config.clientId ? String(config.clientId) : defaultClientId();
  _useAuth = config.username != nullptr;
  _username = config.username ? config.username : "";
  _password = config.password ? config.password : "";

  _baseTopic = config.baseTopic ? String(config.baseTopic) : _clientId;
  while (_baseTopic.endsWith("/")) {
    _baseTopic.remove(_baseTopic.length() - 1);
  }

  _useAvailability = config.useAvailability && config.availabilityTopic != nullptr;
  if (_useAvailability) {
    _availabilityTopic = expandTopic(config.availabilityTopic);
    _payloadOnline = config.payloadOnline ? config.payloadOnline : "online";
    _payloadOffline = config.payloadOffline ? config.payloadOffline : "offline";
  }

  // Subscriptions registered before begin() did not know the base topic yet.
  for (size_t i = 0; i < _subscriptions.size(); i++) {
    _subscriptions[i].filter = expandTopic(_subscriptions[i].pattern.c_str());
  }

  _cleanSession = config.cleanSession;
  _reconnectMinMs = config.reconnectMinMs > 0 ? config.reconnectMinMs : 1;
  _reconnectMaxMs = config.reconnectMaxMs > _reconnectMinMs ? config.reconnectMaxMs : _reconnectMinMs;

  // PubSubClient keeps the pointer, so it must point at our own copy.
  _client.setServer(_host.c_str(), _port);
  _client.setKeepAlive(config.keepAliveSec);
  _client.setSocketTimeout(config.socketTimeoutSec);
  _client.setBufferSize(config.bufferSize);
  _client.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
    handleMessage(topic, payload, length);
  });

  _enabled = true;
  _retryDelayMs = 0;
  return true;
}

void MqttManager::loop() {
  if (!_enabled) {
    return;
  }

  if (_client.connected()) {
    _client.loop();
    return;
  }

  if (_wasConnected) {
    _wasConnected = false;
    _retryDelayMs = 0;  // first retry right away, then back off
    if (_onDisconnect) {
      _onDisconnect();
    }
  }

  if (_networkCheck && !_networkCheck()) {
    return;
  }

  uint32_t now = millis();
  if (_retryDelayMs != 0 && (uint32_t)(now - _lastAttemptMs) < _retryDelayMs) {
    return;
  }
  _lastAttemptMs = now;

  if (attemptConnect()) {
    _wasConnected = true;
    _retryDelayMs = 0;
    if (_useAvailability) {
      _client.publish(_availabilityTopic.c_str(), _payloadOnline.c_str(), true);
    }
    for (size_t i = 0; i < _subscriptions.size(); i++) {
      _client.subscribe(_subscriptions[i].filter.c_str(), _subscriptions[i].qos);
    }
    if (_onConnect) {
      _onConnect();
    }
  } else if (_retryDelayMs == 0) {
    _retryDelayMs = _reconnectMinMs;
  } else {
    uint32_t next = _retryDelayMs * 2;
    _retryDelayMs = (next > _reconnectMaxMs || next < _retryDelayMs) ? _reconnectMaxMs : next;
  }
}

bool MqttManager::attemptConnect() {
  const char* user = _useAuth ? _username.c_str() : nullptr;
  const char* pass = _useAuth ? _password.c_str() : nullptr;

  if (_useAvailability) {
    return _client.connect(_clientId.c_str(), user, pass, _availabilityTopic.c_str(), 1, true,
                           _payloadOffline.c_str(), _cleanSession);
  }
  return _client.connect(_clientId.c_str(), user, pass, nullptr, 0, false, nullptr, _cleanSession);
}

void MqttManager::disconnect() {
  _enabled = false;
  if (_client.connected()) {
    // A clean disconnect suppresses the Last Will, so say goodbye ourselves.
    if (_useAvailability) {
      _client.publish(_availabilityTopic.c_str(), _payloadOffline.c_str(), true);
    }
    _client.disconnect();
  }
  if (_wasConnected) {
    _wasConnected = false;
    if (_onDisconnect) {
      _onDisconnect();
    }
  }
}

void MqttManager::reconnect() {
  if (_host.length() == 0) {
    return;  // begin() was never called successfully
  }
  _enabled = true;
  _retryDelayMs = 0;
}

bool MqttManager::connected() { return _client.connected(); }

int MqttManager::state() { return _client.state(); }

void MqttManager::subscribe(const char* filter, MessageHandler handler, uint8_t qos) {
  addSubscription(filter, qos, handler, nullptr);
}

void MqttManager::subscribeRaw(const char* filter, RawHandler handler, uint8_t qos) {
  addSubscription(filter, qos, nullptr, handler);
}

void MqttManager::addSubscription(const char* filter, uint8_t qos, MessageHandler handler, RawHandler raw) {
  if (filter == nullptr) {
    return;
  }
  Subscription sub;
  sub.pattern = filter;
  sub.filter = expandTopic(filter);
  sub.qos = qos > 1 ? 1 : qos;  // PubSubClient supports QoS 0 and 1 for subscriptions
  sub.handler = handler;
  sub.rawHandler = raw;

  // Same filter again replaces the previous handler.
  bool replaced = false;
  for (size_t i = 0; i < _subscriptions.size(); i++) {
    if (_subscriptions[i].pattern == sub.pattern) {
      _subscriptions[i] = sub;
      replaced = true;
      break;
    }
  }
  if (!replaced) {
    _subscriptions.push_back(sub);
  }

  if (_client.connected()) {
    _client.subscribe(sub.filter.c_str(), sub.qos);
  }
}

void MqttManager::unsubscribe(const char* filter) {
  if (filter == nullptr) {
    return;
  }
  String expanded = expandTopic(filter);
  String pattern(filter);
  for (size_t i = 0; i < _subscriptions.size(); i++) {
    if (_subscriptions[i].pattern == pattern) {
      _subscriptions.erase(_subscriptions.begin() + i);
      break;
    }
  }
  if (_client.connected()) {
    _client.unsubscribe(expanded.c_str());
  }
}

bool MqttManager::publish(const char* topic, const char* payload, bool retain) {
  if (payload == nullptr) {
    payload = "";
  }
  return publish(topic, (const uint8_t*)payload, strlen(payload), retain);
}

bool MqttManager::publish(const char* topic, const String& payload, bool retain) {
  return publish(topic, (const uint8_t*)payload.c_str(), payload.length(), retain);
}

bool MqttManager::publish(const char* topic, const uint8_t* payload, size_t length, bool retain) {
  if (topic == nullptr || !_client.connected()) {
    return false;
  }
  return _client.publish(expandTopic(topic).c_str(), payload, (unsigned int)length, retain);
}

String MqttManager::expandTopic(const char* topic) const {
  if (topic == nullptr) {
    return String();
  }
  if (topic[0] == '~' && topic[1] == '/') {
    return _baseTopic + (topic + 1);
  }
  if (topic[0] == '~' && topic[1] == '\0') {
    return _baseTopic;
  }
  return String(topic);
}

bool MqttManager::topicMatches(const char* filter, const char* topic) {
  if (filter == nullptr || topic == nullptr) {
    return false;
  }
  while (*filter) {
    if (*filter == '#') {
      return true;  // matches the rest, including nothing
    }
    if (*filter == '+') {
      while (*topic && *topic != '/') {
        topic++;
      }
      filter++;
      continue;
    }
    if (*filter != *topic) {
      // "a/#" must also match "a"
      return *topic == '\0' && filter[0] == '/' && filter[1] == '#' && filter[2] == '\0';
    }
    filter++;
    topic++;
  }
  return *topic == '\0';
}

void MqttManager::handleMessage(char* topic, uint8_t* payload, unsigned int length) {
  // Copy first: topic and payload live in PubSubClient's shared buffer, which
  // is overwritten as soon as a handler publishes something.
  String topicCopy(topic);
  String payloadCopy;
  bool payloadCopied = false;

  // Index loop + handler copy so handlers may subscribe/unsubscribe safely.
  for (size_t i = 0; i < _subscriptions.size(); i++) {
    if (!topicMatches(_subscriptions[i].filter.c_str(), topicCopy.c_str())) {
      continue;
    }
    if (_subscriptions[i].rawHandler) {
      RawHandler raw = _subscriptions[i].rawHandler;
      raw(topicCopy.c_str(), payload, length);
    } else if (_subscriptions[i].handler) {
      if (!payloadCopied) {
        payloadCopy.reserve(length);
        for (unsigned int j = 0; j < length; j++) {
          payloadCopy += (char)payload[j];
        }
        payloadCopied = true;
      }
      MessageHandler handler = _subscriptions[i].handler;
      handler(topicCopy, payloadCopy);
    }
  }
}

String MqttManager::defaultClientId() {
  char id[24];
#if defined(ESP32)
  uint64_t mac = ESP.getEfuseMac();
  snprintf(id, sizeof(id), "esp32-%04x%08x", (unsigned)(mac >> 32), (unsigned)(mac & 0xFFFFFFFF));
#elif defined(ESP8266)
  snprintf(id, sizeof(id), "esp8266-%06x", (unsigned)ESP.getChipId());
#else
  snprintf(id, sizeof(id), "mqtt-%08lx", (unsigned long)random(0x7FFFFFFF));
#endif
  return String(id);
}
