#include "led.h"

#include <Arduino.h>
#include "config.h"

static bool ledState = false;

static void ledWrite(bool on) {
  ledState = on;
  digitalWrite(LED_PIN, on ? LED_ON_LEVEL : !LED_ON_LEVEL);
}

void ledBegin() {
  pinMode(LED_PIN, OUTPUT);
  ledWrite(false);
}

void ledOn()   { ledWrite(true); }
void ledOff()  { ledWrite(false); }
bool ledIsOn() { return ledState; }
