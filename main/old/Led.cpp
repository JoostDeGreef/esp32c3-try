#include "Led.h"

Led::Led(const int pin, const bool inverted)
  : pin(pin)
  , state(false)
  , inverted(inverted)
{}

bool Led::Setup()
{
  pinMode(pin, OUTPUT);
  Set(state);
  return true;
}

void Led::On()
{
  Set(true);
}
void Led::Off()
{
  Set(false);
}
void Led::Toggle()
{
  Set(!state);
}
bool Led::State() const
{
  return state;
}

void Led::Set(bool s)
{
  state = s;
  digitalWrite(pin, (state^inverted)?HIGH:LOW);
}

Led OnboardLed(LED_BUILTIN, true);
