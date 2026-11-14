// ============================================================
//  Web servers on the ESP32  -  you do not need to edit this file.
//
//    startServers();   call once in setup(), after connectWiFi()
//
//  They run in the background, nothing to call in loop().
//
//    http://<ip>/             test page with the video and both buttons
//    http://<ip>/button1      toggles output 1
//    http://<ip>/button2      toggles output 2
//    http://<ip>:81/stream    the live video (MJPEG)
//
//  The video is on its own port (81) because it never ends: on
//  port 80 it would block the buttons while someone watches.
// ============================================================
#pragma once

void startServers();
