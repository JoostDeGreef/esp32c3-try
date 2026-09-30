
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"

#include "console.h"
#include "display.h"
#include "serial.h"
#include "joost_onboardled.h"

extern "C" void app_main()
{
    Console::write("Starting up\n");   

    Serial::configure();

    Display::configure();
    
Painter p = Display::getPainter();
p.clear();
p.filled_circle(160,80,50,RGB(128,255,40));
p.circle(120,120,40,RGB(0,255,0));
p.dot(100,100,RGB(0,0,255));
p.line(30,10,200,180,RGB(255,0,0));
p.text(50,150,RGB(128,128,255),"Mira");
Display::flip();

    OnboardLed::flash();

    Console::mainLoop();
}
