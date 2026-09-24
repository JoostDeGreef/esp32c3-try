#pragma once

#include <string>

class Serial
{
public:
    /*
     *  send the message over the serial port
     *  careful, most serial devices have a maximum length!
     */
    static void Send(const std::string & message);

    /*
     *  read whatever data is available on the serial port
     */
    static std::string Read();

    /*
     *  retrieve the serial port configuration
     */
    static std::string Info();

    /*
     *  configure the serial port
     */
    static void configure();
};
