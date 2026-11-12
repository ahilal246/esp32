// ============================================================
//  Web server on the ESP32  -  you do not need to edit this file.
//
//    webServerBegin();   call once in setup(), after wifiBegin()
//
//  It runs in the background, nothing to call in loop().
//  The Python server talks to these addresses (you can also open
//  them in a browser on the same WiFi to test the board):
//
//    http://<ip>/led/on       LED on
//    http://<ip>/led/off      LED off
//    http://<ip>/led          answers "on" or "off"
//    http://<ip>:81/stream    the live video
//
//  The video is on its own port (81) because it never ends: on
//  port 80 it would block the LED commands while someone watches.
// ============================================================
#pragma once

bool webServerBegin();   // false if the server could not start
