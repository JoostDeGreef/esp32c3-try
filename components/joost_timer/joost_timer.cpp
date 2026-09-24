
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"

#include "joost_timer.h"

Timer::Timer(std::function<void()> callback,uint64_t period_ms, bool start_immediately)
    : timer_handle(nullptr)
    , callback(callback)
    , period_ms(period_ms)
{
    esp_timer_create_args_t create_args = {
        .callback = [](void* arg) {
            Timer* timer = static_cast<Timer*>(arg);
            timer->callback();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Joost Timer",
        .skip_unhandled_events = false
    };
    ESP_ERROR_CHECK(esp_timer_create(&create_args, &timer_handle));
    if(start_immediately)
    {
        start();
    }
}

Timer::~Timer()
{
    if(timer_handle != nullptr)
    {
        ESP_ERROR_CHECK(esp_timer_stop_blocking(timer_handle, portMAX_DELAY));
        ESP_ERROR_CHECK(esp_timer_delete(timer_handle));
        timer_handle = nullptr;
    }
}

void Timer::start()
{
    if(timer_handle != nullptr)
    {
       ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, period_ms*1000));
    }
}
void Timer::stop()
{
    if(timer_handle != nullptr)
    {
        ESP_ERROR_CHECK(esp_timer_stop(timer_handle));
    }
}
void Timer::once()
{
    if(timer_handle != nullptr)
    {
       ESP_ERROR_CHECK(esp_timer_start_once(timer_handle, period_ms));
    }
}



