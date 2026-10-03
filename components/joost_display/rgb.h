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
    return ~rgb;
}

// not done
inline void RGB(uint16_t rgb, uint8_t & r, uint8_t & g, uint8_t & b)
{
    rgb = ~rgb;
    r = (rgb >> 8) & 0xF8;
    g = (rgb >> 3) & 0xFC;
    b = (rgb << 3);
}

// not done
inline std::tuple<uint8_t, uint8_t, uint8_t> RGB(uint16_t rgb)
{
    uint8_t r,g,b;
    RGB(rgb,r,g,b);
    return {r, g, b};
}

