// ============================================================
//  Camera helper for the ESP32-CAM (AI Thinker board)
//  You do not need to edit this file.
//
//    cameraBegin();         call once in setup(), after mqttBegin()
//    cameraLoop();          call always in loop(); sends a picture
//                           every CAMERA_INTERVAL_MS (see config.h)
//    cameraSendPicture();   take and send one picture right now
//
//  Pictures go to CAMERA_TOPIC as Base64 text, which is what the
//  phone app's Image panel ("Base64 Payload") displays.
//  Topic, speed and picture size are in config.h
// ============================================================
#pragma once

bool cameraBegin();         // false if no camera was found
void cameraLoop();
bool cameraSendPicture();   // false if not sent
