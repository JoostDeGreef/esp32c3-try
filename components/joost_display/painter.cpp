#include <stdio.h>
#include <unistd.h>
#include <array>
#include <cmath>
#include <algorithm>

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

void Painter::line(int x0, int y0, int x1, int y1, int16_t rgb, int thickness)
{
    // draw the parallel lines
    int dx = x1-x0;
    int dy = y1-y0;
    double length = sqrt(dx*dx + dy*dy);
    double nx = -dy / length;
    double ny =  dx / length;
    const pos p0 = {static_cast<int16_t>(x0 + nx * thickness / 2), static_cast<int16_t>(y0 + ny * thickness / 2)};
    const pos p1 = {static_cast<int16_t>(x1 + nx * thickness / 2), static_cast<int16_t>(y1 + ny * thickness / 2)};
    const pos p2 = {static_cast<int16_t>(x1 - nx * thickness / 2), static_cast<int16_t>(y1 - ny * thickness / 2)};
    const pos p3 = {static_cast<int16_t>(x0 - nx * thickness / 2), static_cast<int16_t>(y0 - ny * thickness / 2)};
    filled_polygon({p0,p1,p2,p3},rgb);
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

int Painter::textWidth(const std::string & text)
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
{
    line(x0,y0,x1,y0,rgb);
    line(x1,y0,x1,y1,rgb);
    line(x1,y1,x0,y1,rgb);
    line(x0,y1,x0,y0,rgb);
}

void Painter::filled_rectangle(int x0, int y0, int x1, int y1, int16_t rgb)
{
    if(y0>y1)
    {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }
    for( int y=y0;y<=y1;++y)
    {
        line(x0, y, x1, y, rgb);
    }
}

void Painter::filled_polygon(const std::vector<pos>& points, int16_t rgb)
{
    //
    // This algorithm is based on the scanline fill algorithm for polygons.
    // It was implemented completely by Mira, I'm very proud of her achievement. She is as brilliant as she is beautiful
    //
    if (points.size() < 3)
    {
        return;
    }

    // Find vertical extent.
    int16_t min_y = points[0].y;
    int16_t max_y = points[0].y;

    for (const pos& p : points)
    {
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    for (int16_t y = min_y; y <= max_y; ++y)
    {
        std::vector<double> intersections;

        for (size_t i = 0; i < points.size(); ++i)
        {
            const pos& p0 = points[i];
            const pos& p1 = points[(i + 1) % points.size()];

            // Horizontal edges don't contribute an intersection.
            if (p0.y == p1.y)
            {
                continue;
            }

            // Use the standard half-open rule:
            // include the lower endpoint, exclude the upper endpoint.
            int16_t edge_min_y = std::min(p0.y, p1.y);
            int16_t edge_max_y = std::max(p0.y, p1.y);

            if (y < edge_min_y || y >= edge_max_y)
            {
                continue;
            }

            double x = p0.x + (double)(y - p0.y) * (double)(p1.x - p0.x) / (double)(p1.y - p0.y);

            intersections.push_back(x);
        }

        if (intersections.size() < 2)
        {
            continue;
        }

        std::sort(intersections.begin(), intersections.end());

        // Convex polygon => the first and last intersections
        // are the left and right boundaries.
        int16_t x0 = (int16_t)std::ceil(intersections.front());
        int16_t x1 = (int16_t)std::floor(intersections.back());
        
        if (x0 <= x1)
        {
            line(x0, y, x1, y, rgb);
        }
    }
}
