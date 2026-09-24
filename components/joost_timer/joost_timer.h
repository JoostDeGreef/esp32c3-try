#pragma once

#include <functional>
#include <memory>
#include "esp_timer.h"

class Timer
{
    public:
        Timer(std::function<void()> callback, uint64_t period_ms = 1000, bool start_immediately = false);
        ~Timer();

        void start();
        void stop();
        void once();

    private:

        esp_timer_handle_t timer_handle;
        std::function<void()> callback;
        uint64_t period_ms;
};
