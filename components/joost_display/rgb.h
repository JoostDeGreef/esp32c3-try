#pragma once

#include <tuple>

inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    // for some reason the display expects the data inverted
    uint16_t rgb =
       ((r & 0xF8) << 8) |
       ((g & 0xFC) << 3) |
       ( b         >> 3);
    return ~rgb;
}

inline void RGB(uint16_t rgb, uint8_t & r, uint8_t & g, uint8_t & b)
{
    rgb = ~rgb;
    r = (rgb >> 8) & 0xF8;
    g = (rgb >> 3) & 0xFC;
    b = (rgb << 3);
}

inline std::tuple<uint8_t, uint8_t, uint8_t> RGB(uint16_t rgb)
{
    uint8_t r,g,b;
    RGB(rgb,r,g,b);
    return {r, g, b};
}

