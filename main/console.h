#pragma once

#include "joost_string.h"

namespace Console 
{
    void write(const char* message);
    template<typename... ARGS>
    void write(const char* format, ARGS... args)
    {
        std::string message = Joost::Format(format, args ...);
        write(message.c_str());
    }

    void mainLoop();
};
