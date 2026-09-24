
#include <esp_timer.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "joost_onboardled.h"

// Define the GPIO pin for the LED 
#define ONBOARDLED_GPIO (gpio_num_t)GPIO_NUM_8

OnboardLed& OnboardLed::getInstance()
{
    static OnboardLed instance;
    return instance;
}

void OnboardLed::on()
{
    getInstance().setState(State::On);
}
void OnboardLed::off()
{
    getInstance().setState(State::Off);
}
void OnboardLed::toggle()
{
    getInstance().setState(getInstance().state() == State::On ? State::Off : State::On);
}
void OnboardLed::blink()
{
    getInstance().setState(State::Blinking);
}
void OnboardLed::flash()
{
    getInstance().setState(State::Flashing);
}
OnboardLed::State OnboardLed::state()
{
    return getInstance().led_state;
}

void OnboardLed::setLed(bool ledOn)
{
    // NOTE: The LED is active low, so we invert the state when setting the GPIO level.
    gpio_set_level(ONBOARDLED_GPIO, (ledOn ? 0 : 1)); 
}
void OnboardLed::setState(State state)
{
    if(this->led_state == state)
    {
        return;
    }
    switch(this->led_state = state)
    {
    case State::On:
        stopBlinking();
        setLed(true);
        break;     
    case State::Off:
        stopBlinking();
        setLed(false);
        break;     
    case State::Blinking:
        startBlinking(1000);
        break;
    case State::Flashing:
        startBlinking(1000, 20);
        break;
    }
}

void OnboardLed::startBlinking(int period_ms, int duty_cycle_percent)
{
    int active_ms = (period_ms * duty_cycle_percent) / 100;
    blink_timer = std::unique_ptr<Timer>(new Timer([period_ms, active_ms, this]()
    {
        setLed(true);
        vTaskDelay(pdMS_TO_TICKS(active_ms));
        setLed(false);
    }, period_ms));
    blink_timer->start();
}
void OnboardLed::stopBlinking()
{
    blink_timer.reset();
}

OnboardLed::OnboardLed()
    : blink_timer(nullptr)
{
    gpio_reset_pin(ONBOARDLED_GPIO);
    gpio_set_direction(ONBOARDLED_GPIO, GPIO_MODE_OUTPUT);            
}



