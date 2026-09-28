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

class Painter
{
    friend class Display;
public:
    void clear();
    void fill(int16_t rgb);
    void dot(int x, int y, int16_t rgb);
    void line(int x0, int y0, int x1, int y1, int16_t rgb);

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

class Display
{
public:

    /*
     *  configure the display
     */
    static void configure();

    /*
     *  flip the buffers, sending the previously painted one to the display
     */
    static void flip();

    /*
     *  get a painter for the currently active buffer 
     */
    static Painter getPainter();
};
