#pragma once

#include "joost_timer.h"

class OnboardLed
{
    public:
        static OnboardLed& getInstance();

        static void on();
        static void off();
        static void toggle();
        static void blink();
        static void flash();

        enum class State
        {
            On,
            Off,
            Blinking,
            Flashing
        };

        static State state();

    private:
        OnboardLed();
        OnboardLed(const OnboardLed&) = delete;
        OnboardLed& operator=(const OnboardLed&) = delete;

        void setLed(bool ledOn); // led on or off, doesn't touch state
        void setState(State state); 

        void startBlinking(int period_ms = 1000, int duty_cycle_percent = 50);
        void stopBlinking();

        std::unique_ptr<Timer> blink_timer;
        State led_state; 
    };
