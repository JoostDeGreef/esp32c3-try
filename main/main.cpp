
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"

#include "clock.h"
#include "console.h"
#include "serial.h"
#include "joost_onboardled.h"

extern "C" void app_main()
{
    Console::write("Starting up\n");   

    Serial::configure();
  
    Clock::start();

    OnboardLed::flash();

    Console::mainLoop();
}
