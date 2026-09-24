
#include "Timer.h"

using namespace std;
using namespace Timer_internal;

bool Timer::Setup()
{
  return true;
}

void Timer::AddEvent(std::function<void()> f, unsigned long delay)
{
  events.emplace_back(f, delay);
}

void Timer::Tick()
{
  unsigned long ticks = millis();
  for(Event & event: events)
  {
    event.Tick(ticks);
  }
}

::Timer_internal::Timer Timer;
