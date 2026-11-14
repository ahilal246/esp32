#include "buttons.h"

#include <Arduino.h>
#include "config.h"

static bool button1State = false;
static bool button2State = false;

void buttonsBegin() {
  // Outputs start in the OFF state.
  pinMode(BUTTON1_PIN, OUTPUT);
  digitalWrite(BUTTON1_PIN, !BUTTON1_ON);
  pinMode(BUTTON2_PIN, OUTPUT);
  digitalWrite(BUTTON2_PIN, !BUTTON2_ON);
}

// Each press flips the output. Put your own action here if you need more
// than a pin going on and off.
bool toggleButton1() {
  button1State = !button1State;
  digitalWrite(BUTTON1_PIN, button1State ? BUTTON1_ON : !BUTTON1_ON);
  Serial.printf("Button 1 -> %s\n", button1State ? "ON" : "OFF");
  return button1State;
}

bool toggleButton2() {
  button2State = !button2State;
  digitalWrite(BUTTON2_PIN, button2State ? BUTTON2_ON : !BUTTON2_ON);
  Serial.printf("Button 2 -> %s\n", button2State ? "ON" : "OFF");
  return button2State;
}
