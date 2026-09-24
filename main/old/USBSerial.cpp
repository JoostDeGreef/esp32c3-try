
#include "Output.h"
#include "USBSerial.h"

using namespace std;
using namespace internal;

// this always returns true.
// not being connected to USB is annoying for debugging, but not a failure 'in the field'.
bool USBSerial::Setup()
{
  Serial.begin(BaudRate);
  Serial.setRxBufferSize(RxBufferSize);
  Serial.setTxBufferSize(TxBufferSize);
  // Wait .5 s for proper startup
  unsigned long start = millis();
  while (!Serial && millis() - start < 500) {}
  if (Serial) 
  {
    Serial.print("Serial output started" CRLF);
  }
  return true;
}

string USBSerial::ReadInput() const
{
  // caches partially received commands. 
  // the line is considered complete when \0, \r or \n is received
  static string buffer;
  string result;
  if(Serial)
  {
    buffer.reserve(128);
    while (Serial.available())
    {
      char t = Serial.read();
      if( t=='\n' || t=='\0' || t=='\r')
      {
        result.swap(buffer);
        break;
      }
      buffer.push_back(t);
    }
  }
  return result;
}

void USBSerial::SendString(std::string s) const
{
  if (!Serial)
  {
      return;
  }
  constexpr char prefix[] = "Message truncated: ";
  constexpr size_t prefixLength = sizeof(prefix) - 1;

  int bytesAvailable = Serial.availableForWrite();
  if(s.size()>bytesAvailable)
  {
    if (bytesAvailable <= prefixLength)
    {
        return;
    }
    Serial.print(prefix);
    bytesAvailable -= prefixLength;
    if (s.size() > static_cast<size_t>(bytesAvailable))
    {
        s.resize(bytesAvailable);
    }
  }
  Serial.print(s.c_str());
}

::internal::USBSerial USBSerial;
