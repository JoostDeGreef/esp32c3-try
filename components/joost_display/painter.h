#pragma once

#include <tuple>
#include <string>

#include "rgb.h"

struct pos
{
    int16_t x;
    int16_t y;
};

class Painter
{
    friend class Display;
public:
    void clear();
    void fill(int16_t rgb);
    void dot(int x, int y, int16_t rgb);
    void dot(const pos & p, int16_t rgb) { dot(p.x, p.y, rgb); }
    void line(int x0, int y0, int x1, int y1, int16_t rgb);
    void line(const pos & p0, const pos & p1, int16_t rgb) { line(p0.x,p0.y,p1.x,p1.y,rgb); }
    void line(int x0, int y0, int x1, int y1, int16_t rgb, int thickness);
    void line(const pos & p0, const pos & p1, int16_t rgb, int thickness) { line(p0.x,p0.y,p1.x,p1.y,rgb,thickness); }
    void circle(int cx, int cy, int radius, int16_t rgb);
    void circle(const pos & p, int radius, int16_t rgb) { circle(p.x,p.y,radius,rgb); }
    void filled_circle(int cx, int cy, int radius, int16_t rgb);
    void filled_circle(const pos & p, int radius, int16_t rgb) { filled_circle(p.x,p.y,radius,rgb); }
    void rectangle(int x0, int y0, int x1, int y1, int16_t rgb);
    void filled_rectangle(int x0, int y0, int x1, int y1, int16_t rgb);
    void text(int x, int y, int16_t rgb, const std::string & txt);
    void text(const pos & p, int16_t rgb, const std::string & txt) { text(p.x,p.y,rgb,txt); }
    int textWidth(const std::string & text);

protected:    
private:
    Painter(uint16_t width, uint16_t height, uint16_t * buffer)
        : buffer(buffer)
        , width(width)
        , height(height)
    {}

    inline constexpr int index(int x, int y)
    {
        if(x<0 || x>=width || y<0 || y>=height)
        {
            // error!
            return -1;
        }
        return y*width+x;
    }

    uint16_t * buffer;
    uint16_t width;
    uint16_t height;
};

