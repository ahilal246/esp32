// ============================================================
//  WiFi helper  -  you do not need to edit this file.
//
//    connectWiFi();   call once in setup(). Waits until the board
//                     is connected. After 30 seconds without
//                     success the board restarts and tries again.
//
//  WiFi name, password and the optional fixed address are in config.h
//  (This file is not called "wifi.h" because that clashes with the
//   ESP32's own WiFi.h on Windows and macOS.)
// ============================================================
#pragma once

void connectWiFi();
