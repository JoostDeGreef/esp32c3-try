#include <stdio.h>
#include <unistd.h>
#include <array>

#include "painter.h"
#include "font_data.h"

using namespace std;

void Painter::clear()
{
    fill(RGB(0,0,0));
}

void Painter::fill(int16_t rgb)
{
    for(uint16_t i = 0; i<width*height; ++i)
    {
        buffer[i] = rgb;
    }
}

void Painter::dot(int x, int y, int16_t rgb)
{
    auto i = index(x,y);
    if(i>=0)
    {
        buffer[i] = rgb;
    }
}

void Painter::line(int x0, int y0, int x1, int y1, int16_t rgb)
{
    int dx = x1-x0;
    int dy = y1-y0;
    auto swapPointsIfNeeded = [&](bool swap)
    {
        if(swap)
        {
            std::swap(x0,x1);
            std::swap(y0,y1);
            dx = x1-x0;
            dy = y1-y0;
        }
    };
    if(abs(dx)>abs(dy))
    {
        swapPointsIfNeeded(dx<0);
        for(int i=0;i<=dx;++i)
        {
            int j = (dy*i)/dx;
            dot(x0+i,y0+j,rgb);
        }
    }
    else
    {
        swapPointsIfNeeded(dy<0);
        for(int i=0;i<=dy;++i)
        {
            int j = (dx*i)/dy;
            dot(x0+j,y0+i,rgb);
        }
    }
}

void Painter::circle(int cx, int cy, int radius, int16_t rgb)
{
    int x = radius;
    int y = 0;
    int decision = 1 - radius;

    while (x >= y)
    {
        dot(cx + x, cy + y, rgb);
        dot(cx + y, cy + x, rgb);
        dot(cx - y, cy + x, rgb);
        dot(cx - x, cy + y, rgb);
        dot(cx - x, cy - y, rgb);
        dot(cx - y, cy - x, rgb);
        dot(cx + y, cy - x, rgb);
        dot(cx + x, cy - y, rgb);

        ++y;
        if (decision <= 0)
        {
            // this follows from (y+1)^2 − y^2 = 2y + 1
            decision += 2 * y + 1;
        }
        else
        {
            --x;
            decision += 2 * (y - x) + 1;
        }
    }
}

void Painter::filled_circle(int cx, int cy, int radius, int16_t rgb)
{
    int x = radius;
    int y = 0;
    int decision = 1 - radius;

    while (x >= y)
    {
        line(cx - x, cy + y, cx + x, cy + y, rgb);
        line(cx - x, cy - y, cx + x, cy - y, rgb);
        line(cx - y, cy + x, cx + y, cy + x, rgb);
        line(cx - y, cy - x, cx + y, cy - x, rgb);

        decision += (decision <= 0) ? 2 * (++y) + 1 : 2 * ((++y) - (--x)) + 1;
    }
}

void Painter::text(int x, int y, int16_t rgb, const std::string & text)
{
    auto drawChar = [&](const char curr, const char next)
    {
        const auto & c = font_data[curr];
        int i=0;
        int xo = c.offset_x;
        int yo = c.offset_y;
        for(int yi=0;yi<c.height;++yi)
        {
            for(int xi=0;xi<c.width;++xi)
            {
                if(c.data[i/8] & (1<<(i % 8)))
                {
                    dot(x+xi+xo,y+yi+yo,rgb);
                }
                ++i;
            }
        }
        x += c.advance;
        auto it = font_kernings.find({curr, next});
        if(it != font_kernings.end())
        {
            x += it->second;
        }
    };
    for(int i=0;i<text.size();++i)
    {
        const char curr = text[i];
        const char next = i+1 < text.size() ? text[i+1] : 0;
        drawChar(curr, next);
    }
}

int Painter::textWidth(int x, int y, const std::string & text)
{
    int res = 0;
    for(int i=0;i<text.size();++i)
    {
        const char curr = text[i];
        const char next = i+1 < text.size() ? text[i+1] : 0;
        res += font_data[curr].advance;
        auto it = font_kernings.find({curr, next});
        if(it != font_kernings.end())
        {
            res += it->second;
        }
    }
    return res;
}

void Painter::rectangle(int x0, int y0, int x1, int y1, int16_t rgb)
{}
void Painter::filled_rectangle(int x0, int y0, int x1, int y1, int16_t rgb)
{}
