// ============================================================
//  MPU6050 wheel speed helper  -  you do not need to edit this file.
//
//    mpuBegin();     call once in setup(). Keep the wheel STILL for a
//                    few seconds while it measures the gyro's zero point
//    mpuLoop();      call always in loop(): reads the sensor 100x a second
//    wheelRpm();     wheel speed in turns per minute (smoothed)
//    mpuWorking();   false if the sensor doesn't answer (check the wires)
//
//  It uses the gyroscope, which measures how fast the sensor turns,
//  so the RPM comes straight from it. The sensor can sit anywhere on
//  the wheel, facing any direction. It reads up to about 330 RPM.
//
//  Pins and address are in config.h. No extra library needed.
// ============================================================
#pragma once

bool  mpuBegin();
void  mpuLoop();
float wheelRpm();
bool  mpuWorking();
