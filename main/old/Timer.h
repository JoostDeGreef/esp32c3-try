#pragma once

#include <functional>
#include <vector>

namespace Timer_internal
{
  class Timer final
  {
    public:
      bool Setup();

      void AddEvent(std::function<void()> f, unsigned long delay);

      void Tick();
    private:
      struct Event
      {
        Event(std::function<void()> f, unsigned long delay)
          : last(millis())
          , delay(delay)
          , f(f)
        {}
        void Tick(unsigned long ticks)
        {
          if(ticks - last >= delay)
          {
            last = ticks;
            f();
          }
        }
        unsigned long last;
        unsigned long delay;
        std::function<void()> f;
      };
      std::vector<Event> events;
  };
}

extern ::Timer_internal::Timer Timer;
