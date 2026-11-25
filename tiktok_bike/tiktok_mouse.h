// ============================================================
//  Bluetooth mouse helper  -  you do not need to edit this file.
//
//    mouseBegin();        call once in setup(): the phone can now find BLE_NAME
//    mousePaired();       true while the phone is connected and paired
//    mouseNextVideo();    scrolls down = next TikTok video
//    mouseParkCursor();   pushes the pointer to the right edge of the screen
//
//  Name and scroll steps are in config.h
//  Needs the "HijelHID_BLEMouse" and "NimBLE-Arduino" libraries.
// ============================================================
#pragma once

void mouseBegin();
bool mousePaired();
void mouseNextVideo();
void mouseParkCursor();
