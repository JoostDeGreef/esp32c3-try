
#include "Output.h"
#include "USBSerial.h"

using namespace std;
using namespace Output_internal;

Output::Output()
{}

bool Output::Setup()
{
  return true;
}

void Output::WriteString(const string & s) const
{
  ::USBSerial.SendString(s);
}

::Output_internal::Output Output;

