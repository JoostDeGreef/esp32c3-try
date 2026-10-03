#pragma once

#include <tuple>

//
// This is a very specific rgb->bgr565 function for the GC9A01 module i'm using. It is not a generic function
//
inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t rgb =
       ((r & 0xF8) << 8) |
       ((g & 0xFC) << 3) |
       ( b         >> 3);
    //return ~rgb; // esp is inverted?
    return (rgb << 8) | (rgb >> 8); // adafruit style
}


