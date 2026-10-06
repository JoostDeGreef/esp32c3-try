#pragma once

#include <tuple>
#include <string>

#include "rgb.h"
#include "painter.h"

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

    /*
     *  state of the display go to sleep state
     */
    static void sleep(bool s);
};
