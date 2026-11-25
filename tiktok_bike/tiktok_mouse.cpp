#include "tiktok_mouse.h"

#include <Arduino.h>
#include <HijelHID_BLEMouse.h>
#include "config.h"

static HijelBLEMouse mouse(BLE_NAME, BLE_MAKER);

void mouseBegin() { mouse.begin(); }

bool mousePaired() { return mouse.isPaired(); }

void mouseNextVideo() {
  mouse.addScroll(-SCROLL_STEPS);   // negative = scroll down = next video
}

void mouseParkCursor() {
  // Moves the pointer 3000 to the right (24 x 127), like the old
  // moveTo(3000, 0). moveTo() moves FROM where the pointer is, so this
  // ends at the right edge. It is done in small steps here because
  // moveTo() waits forever if the phone disconnects halfway through.
  for (int i = 0; i < 24 && mouse.isPaired(); i++) {
    mouse.move(127, 0);
    delay(20);   // the library sends one step about every 7 ms
  }
}
