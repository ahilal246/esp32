// ============================================================
//  The two outputs that the web buttons switch.
//  You do not need to edit this file.
//
//    buttonsBegin();    call once in setup(), both outputs start OFF
//    toggleButton1();   flip output 1, returns true if it is now ON
//    toggleButton2();   flip output 2, returns true if it is now ON
//
//  The pins are in config.h
// ============================================================
#pragma once

void buttonsBegin();
bool toggleButton1();
bool toggleButton2();
