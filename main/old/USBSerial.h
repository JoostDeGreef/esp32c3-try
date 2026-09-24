#pragma once

#include <string> 

#include "Support.h"

namespace internal
{
  //
  // Class which is responsible for serial input/output
  // There can be only one instance, so use the global USBSerial object
  //
  class USBSerial final
  {
  public:
    bool Setup();

    std::string ReadInput() const;
    void SendString(std::string s) const;

  private:
    int BaudRate = 115200; 
    // set a big buffer size so we can send/receive complex messages (commands with payload)
    int RxBufferSize = 2048;
    int TxBufferSize = 2048;
  };
}

extern internal::USBSerial USBSerial;
