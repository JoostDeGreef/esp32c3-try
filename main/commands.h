#pragma once

namespace Commands 
{
    void registerCommands();    
    void getCompletion(const char *buf, linenoiseCompletions *lc);
    char *getHint(const char *buf, int *color, int *bold);
    void runCommand(const char *line);
}
