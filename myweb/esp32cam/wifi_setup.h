// ============================================================
//  WiFi helper  -  you do not need to edit this file.
//
//    wifiBegin();       call once in setup(), waits until connected
//                       and prints the board's IP address
//    wifiConnected();   true while WiFi is up
//
//  WiFi name and password are in config.h
//  (This file is not called "wifi.h" because that clashes with the
//   ESP32's own WiFi.h on Windows and macOS.)
// ============================================================
#pragma once

void wifiBegin();
bool wifiConnected();
