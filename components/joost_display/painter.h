#pragma once

#include <tuple>
#include <string>

#include "rgb.h"

class Painter
{
    friend class Display;
public:
    void clear();
    void fill(int16_t rgb);
    void dot(int x, int y, int16_t rgb);
    void line(int x0, int y0, int x1, int y1, int16_t rgb);
    void circle(int cx, int cy, int radius, int16_t rgb);
    void filled_circle(int cx, int cy, int radius, int16_t rgb);
    void rectangle(int x0, int y0, int x1, int y1, int16_t rgb);
    void filled_rectangle(int x0, int y0, int x1, int y1, int16_t rgb);
    void text(int x, int y, int16_t rgb, const std::string & text);
    int textWidth(int x, int y, const std::string & text);

protected:    
private:
    Painter(uint16_t width, uint16_t height, uint16_t * buffer)
        : buffer(buffer)
        , width(width)
        , height(height)
    {}

    inline constexpr int index(int x, int y)
    {
    #ifdef DEBUG
        if(x<0 || x>=width || y<0 || y>=height)
        {
            // error!
            return -1
        }
    #endif
        return y*width+x;
    }

    uint16_t * buffer;
    uint16_t width;
    uint16_t height;
};

