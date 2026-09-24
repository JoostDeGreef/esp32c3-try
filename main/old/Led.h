#pragma once

#undef LED_BUILTIN
#define LED_BUILTIN 8 // Internal LED pin is 8 as per schematic

//
// Small class for operating a led connected to a gpio
//
class Led final
{
public:
  Led(const int pin, const bool inverted = false);

  bool Setup();

  void On();
  void Off();
  void Toggle();
  bool State() const;
private:
  void Set(bool s);

  int pin;
  bool state;
  bool inverted;
};

extern Led OnboardLed;
