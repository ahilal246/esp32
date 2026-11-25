// ============================================================
//  MPU6050 wheel speed helper  -  you do not need to edit this file.
//
//    mpuBegin();   call once in setup(). Keep the wheel STILL for a
//                  few seconds while it calibrates the gyro
//    mpuLoop();    call always in loop()
//    wheelRpm();   wheel speed in turns per minute
//
//  RPM = square root of (gx*gx + gy*gy + gz*gz) / 6
//  so the sensor can face any direction on the wheel.
//
//  Pins are in config.h. Needs the "MPU6050_light" library.
// ============================================================
#pragma once

bool  mpuBegin();
void  mpuLoop();
float wheelRpm();
