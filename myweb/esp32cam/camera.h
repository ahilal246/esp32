// ============================================================
//  Camera helper for the ESP32-CAM (AI Thinker board)
//  You do not need to edit this file.
//
//    cameraBegin();                call once in setup()
//    cameraTakePicture();          take one picture (a JPEG)
//    cameraReturnPicture(picture); give it back when you are done
//
//  Every picture you take MUST be returned, or the camera runs
//  out of memory after a few pictures:
//
//    camera_fb_t* picture = cameraTakePicture();
//    if (picture != nullptr) {
//      // picture->buf = the JPEG bytes, picture->len = how many
//      cameraReturnPicture(picture);
//    }
//
//  Picture size and quality are in config.h
// ============================================================
#pragma once

#include <esp_camera.h>

bool cameraBegin();                 // false if no camera was found
camera_fb_t* cameraTakePicture();   // nullptr if it did not work
void cameraReturnPicture(camera_fb_t* picture);
