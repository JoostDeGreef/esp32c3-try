#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cstring>  

#include "linenoise/linenoise.h"

//#include "ansi.h"
#include "commands.h"
#include "console.h"
#include "serial.h"
#include "joost_onboardled.h"
#include "joost_string.h"

struct CommandData
{
    std::vector<std::string> names;    // name and possibly aliases, for instance "help,?"
    std::string description;           // short one line description
    std::string hint;                  // short one line arguments listing
    std::string info;                  // full info for the command
    void (*handler)(const char* args); // command handler
};

class CommandsImpl
{
public:
    static CommandsImpl& getInstance(); 

    void registerCommands();
    void getCompletion(const char *buf, linenoiseCompletions *lc);
    char *getHint(const char *buf, int *color, int *bold);
    void runCommand(const char *line);
private:
    CommandsImpl()
        : commands()
        , commandMap()
    {}
    CommandsImpl(const CommandsImpl&) = delete;
    CommandsImpl& operator=(const CommandsImpl&) = delete;  

    void rebuildCommandMap();
    int findCommandIndex(const std::string& cmd) const;

    std::vector<CommandData> commands;
    std::unordered_multimap<std::string, int> commandMap;
};

void Commands::registerCommands()
{
    CommandsImpl::getInstance().registerCommands();
}

void Commands::getCompletion(const char *buf, linenoiseCompletions *lc)
{
    CommandsImpl::getInstance().getCompletion(buf, lc);
}

char *Commands::getHint(const char *buf, int *color, int *bold)
{
    return CommandsImpl::getInstance().getHint(buf, color, bold);
}

void Commands::runCommand(const char *line)
{
    CommandsImpl::getInstance().runCommand(line);
}

CommandsImpl& CommandsImpl::getInstance()
{
    static CommandsImpl instance;
    return instance;
}

void CommandsImpl::registerCommands()
{
    commands.clear();

    commands.push_back({
        {"help","?"}, 
        "Display this help message", 
        " help <command>", 
        "More information on each command is available by using \033[0;33m\"help <command>\"\033[0m",
        [](const char* args) 
    {
        if(args == nullptr || strlen(args)==0 )
        {
            Console::write("\n\033[0;36m=== Available commands ===\033[0m\n");
            for (const auto& cmd : CommandsImpl::getInstance().commands) 
            {
                Console::write("\033[0;33m%s\033[0m: %s\n", Joost::Join(cmd.names,",").c_str(), cmd.description.c_str());
            }
            Console::write("For more information, use 'help <command>'\n");
        }
        else
        {
            auto & instance = CommandsImpl::getInstance();
            int index = instance.findCommandIndex(args);
            switch(index)
            {
                case -1: // No command found
                    Console::write("\n\033[0;31mCommand '%s' not found\033[0m\n", args);
                    break;
                case -2: // Multiple commands found
                    Console::write("\n\033[0;31mCommand abbreviation '%s' matches multiple commands\033[0m\n", args);
                    break;
                default: // Single command found
                    {
                        auto & cmd = instance.commands[index];
                        Console::write("\n\033[0;36m=== Help for %s ===\033[0m\n", cmd.names.front().c_str());
                        Console::write("\033[0;33m%s\033[0m: %s\n", Joost::Join(cmd.names,",").c_str(), cmd.description.c_str());
                        Console::write("   Arguments: \033[0;33m%s\033[0m\n%s\n", cmd.hint.c_str(), cmd.info.c_str());
                    }
                    break;
            }

        }
    }});

    commands.push_back({
        {"echo"}, 
        "Echo the input arguments", 
        " echo [message]", 
        "Output (echo) the message on the local device",
        [](const char* args) 
    {
        Console::write("%s\n", args);
    }});

    commands.push_back({
        {"copyright"}, 
        "Show Copyright information", 
        " copyright", 
        "Show Copyright information",
        [](const char* args) 
    {
        Console::write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡤⣤⠆⠀⢲⡤⢤⣄\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⣦⠀⠀⠀⢸⣇⢻⡇⠀⣸⡇⣾⠃⢠⣶⡉⠱\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⡄⠹⣧⡀⠀⠸⠇⠘⠓⠀⠛⠃⠘⠦⢻⣇⠘⠀⠀⠀⣠⠕⣄⠀\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠰⠛⠻⣦⠈⠿⠶⠋⢁⣀⣤⣶⣶⣾⣿⣿⣶⣶⣦⣤⣀⠀⠀⢴⠟⢣⣼⠟⠁⢠\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣄⠀⢻⠀⠀⢀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⠈⠀⢾⠋⠀⣠⣾⠦\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠚⡄⠀⠙⠛⠋⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠐⢿⠋⠀⠀⢀⡀\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⠛⠛⠛⢷⠆⠀⣤⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣄⠈⢡⣴⠾⠛⠁\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣆⠀⠧\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⣿⣿⣿⣿⣿⠟⠉⠀⠀⠀⠈⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⠃⣀⣶⣶⣶⣦⡈⠀⠈⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⠰⡿⣿⣿⢛⠛⣿⡄⠀⠀⠙⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡯\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⡄⠀⠀⠀⢿⣿⣿⣿⣿⣿⠄⢀⠄⢷⣇⢉⣬⣿⠘⠀⠀⠀⣹⣿⣿⣿⣿⣿⣿⣿⣿⡇\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⠀⠀⠀⠀⠘⣿⣿⣿⣿⡟⠀⠸⣴⠸⡽⠻⢿⡟⠀⢡⡀⠀⠀⠙⢻⣿⣿⣿⣿⣿⣿⠁⠀⠀⠠⡧⠄\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠹⣿⣿⣿⡧⣠⣤⡈⠀⡠⢰⡆⠀⠀⢸⢿⡟⢀⠂⠄⡄⢹⣻⣿⣿⠁⠀⠀⠀⠀⠁\n⠀⠀⠀⠀⠀⠀⠀⠀⠒⡗⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣭⣭⢭⣾⣿⣿⣷⡀⠉⠀⠀⠀⠀⣜⡾⠀⡐⡈⡐⢀⢺⣿⣮⢡⣴⣶⣤⣄⡀⠀⠀⠀⠀⠐⠒\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣿⣿⣿⣵⣿⣿⢻⡿⣿⣿⣦⣀⡠⢤⣎⣼⠃⢁⠐⠄⡘⠀⢾⢿⣿⣷⣝⣿⣿⣿⣷⣆⢀⣀⣀⣀\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⣿⣿⣿⣿⣿⣿⣫⣿⣿⣿⣿⢸⡳⣿⣿⣎⢷⣿⣿⣿⠇⠀⠂⠌⠐⡀⠁⣾⣿⡿⣿⡿⠷⣛⣛⣿⡻⣿⣿⣿⣿⣷⣀⣀⣀⡀\n⠀⠀⠀⠀⠀⠀⢠⣀⣀⣄⠠⢿⣿⣿⣽⣾⣗⣲⣭⣽⣿⣿⣿⡿⢸⣿⢻⡿⣿⣧⣻⢿⠇⠀⠌⠂⣈⠒⢀⢱⣇⣻⢿⣿⣿⣿⣿⣿⣿⣿⣶⣯⣭⣭⣭⠽⠿⢿⡛⠂\n⠀⠀⠀⠀⣐⠖⠒⢲⣍⣫⣭⡇⢻⣽⣿⣿⣿⣿⢿⣿⣿⣿⡟⣹⢸⣿⡼⣿⣿⣿⣿⠃⠀⡰⢈⠂⠤⡈⠀⣽⣿⣰⡹⣿⣿⡿⠿⣿⠿⢿⣟⣽⣿⢫⡔⠿⢛⣥⣀\n⠀⠀⠀⠀⠈⠉⠐⠚⠉⠈⠀⣿⡎⣿⣷⣯⣛⣻⠿⣿⣿⢟⣼⣽⣾⣿⣥⣙⢿⡿⠃⢠⠘⡀⠆⠉⣐⣠⣤⣭⣝⡛⡟⣝⢿⣿⣿⣿⣿⣿⣿⣿⢣⣹⡿⠯⢝⠯⣂⠯⣂\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣧⣿⣿⣿⣿⣿⣿⡿⣫⣿⣿⣿⣿⣿⣿⣿⣷⡄⢘⡀⠆⡁⣠⣾⣿⣿⣿⣿⣿⣿⣾⣿⣷⡝⢿⣿⣿⣿⣿⣿⢼⣿⣿⣿⡆⠈⠀⠉\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣻⣿⣿⣿⢟⣯⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣴⣶⣶⣶⣭⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣝⢿⣿⣿⣿⢸⣿⣿⣿\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢟⡽⠿⠋⠀⠙⠻⠿⠿⠿⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⣿⣿⣿⣿⣿⣿⠿⠋⠉⠛⠋⠛⠿⣧⢿⣿⡏\n⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠙⠉⠉⠁⠀⠉⠉⠉⠉⠉⠀⠀⠀⠉⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉\n⠀⠀⠀⠀⠀⣤⠠⣄⠀⢤⡄⠀⠤⡤⠀⡄⢤⡄⠠⣤⢤⡄⣠⡄⢤⡀⢀⠤⣄⠀⣤⡄⢠⣤⠀⣤⢠⢤⡤⠤⢀⣤⠀⣤⡄⠀⡠⠤⡄⣤⡄⠠⡄⠤⣤⡄⠄⠠⢤⠄\n⠀⠀⠀⠀⠀⣿⠀⢹⠇⢸⡇⠘⢷⣤⢰⡧⢼⡇⠐⣿⠘⠃⢸⡇⠞⢱⣿⠀⣿⠀⣿⡇⢸⠹⣦⡇⠀⢸⡇⠀⢸⢹⣖⢹⡇⢸⡇⠰⠀⠈⢻⣤⡇⠀⣿⡇⠀⠻⣦⡄\n⠀⠀⠀⠀⠀⠿⡤⠞⠀⠼⢇⢤⡬⠋⠼⠇⠸⠇⠰⢿⠀⠀⡼⢇⠀⠘⠿⡤⠛⠀⠿⠇⠸⠄⠹⢧⠀⠸⢇⠀⠸⠸⠛⠸⢧⠘⢧⡤⠀⠴⡈⠿⢇⠀⠿⠇⠠⣤⠝⠁\n");
        Console::write("\n\033[0;31m(C) 2026 Andreas de Greef & et patre eius\033[0m\n");
    }});

    commands.push_back({
        {"led"}, 
        "Control the onboard LED", 
        " led <on|off|toggle|blink|flash>", 
        "Query or change the current led state. possible actions:\n"
        "  on     : switch led on\n"
        "  off    : switch led off\n"
        "  toggle : toggle led state\n"
        "  blink  : switch blinking mode on\n"
        "  flash  : switch flashing mode on\n"
        "Without any arguments, the current led state is displayed",
        [](const char* args) 
    {
        std::string cmd = Joost::ToLower(args);
        if (cmd == "on") 
        {
            OnboardLed::on();
        } 
        else if (cmd == "off") 
        {
            OnboardLed::off();
        }
        else if (cmd == "toggle") 
        {
            OnboardLed::toggle();
        }
        else if (cmd == "blink") 
        {
            OnboardLed::blink();
        }
        else if (cmd == "flash") 
        {
            OnboardLed::flash();
        }
        else if (cmd.empty()) 
        {
            static std::map<OnboardLed::State, std::string> states = 
            {
                {OnboardLed::State::On, "on"},
                {OnboardLed::State::Off, "off"},
                {OnboardLed::State::Blinking, "blinking"},
                {OnboardLed::State::Flashing, "flashing"},
            };
            Console::write("led is %s\n", states[OnboardLed::state()].c_str());
        }
        else
        {
            Console::write("\n\033[0;31m\"led '%s'\" is not a valid command\033[0m\n", cmd.c_str());
        }
    }});

    commands.push_back({
        {"send","transmit"}, 
        "Send message to serial port", 
        " send [message]", 
        "Send (transmit) the message to the serial port on the local device",
        [](const char* args) 
    {
        Serial::Send(args);
    }});

    commands.push_back({
        {"info"}, 
        "Show device information / configuration", 
        " info", 
        "Show the current device information / configuration",
        [](const char* args) 
    {
        Console::write("\033[0;34mSerial port config:\n%s\033[0m\n",Serial::Info().c_str());
    }});

    commands.push_back({
        {"read","receive"}, 
        "Read message available on serial port (if any)", 
        " read", 
        "Read (receive) any message available on the serial port on the local device",
        [](const char* args) 
    {
        Console::write("\033[0;34mReceived \"%s\"\033[0m\n",Serial::Read().c_str());
    }});

    // make sure the commands are sorted alphabetically in 'help' output
    std::sort(commands.begin(), commands.end(), [](const CommandData& a, const CommandData& b) 
    {
        return a.names.front() < b.names.front();
    });

    rebuildCommandMap();
}

void CommandsImpl::rebuildCommandMap()
{
    commandMap.clear();
    for (size_t i = 0; i < commands.size(); ++i) 
    {
        for (size_t j = 0; j < commands[i].names.size(); ++j) 
        {
            const auto& name = commands[i].names[j];
            const int index = (i << 8) + j; // Store both command index and alias index in a single integer
            for(size_t k = 1; k <= name.size(); ++k)
            {
                std::string prefix = name.substr(0, k);
                commandMap.emplace(prefix, index);
            }
        }
    }
}

void CommandsImpl::getCompletion(const char *buf, linenoiseCompletions *lc)
{
    if(nullptr == buf || strlen(buf) == 0)
    {
        return;
    }
    std::string cmd = Joost::ToLower(Joost::TrimCopy(buf));
    for (auto iter = commandMap.find(cmd); iter != commandMap.end() && iter->first == cmd; iter++) 
    { 
        int alias = iter->second & 0xFF; // Extract alias index
        int index = iter->second >> 8; // Extract command index
        linenoiseAddCompletion(lc, commands[index].names[alias].c_str());
    }
}

int CommandsImpl::findCommandIndex(const std::string& cmd) const
{
    auto range = commandMap.equal_range(cmd);
    auto iter = range.first;
    int index = -1;
    if (iter != range.second) 
    {
        index = iter->second >> 8; 
        for(;iter != range.second; ++iter)
        {
            int other_index = iter->second >> 8; // Extract command index
            if(other_index!=index)
            {
                index = -2;
                break;
            }
        }
    }
    return index;
}

char *CommandsImpl::getHint(const char *buf, int *color, int *bold)
{
    if(nullptr == buf || strlen(buf) == 0)
    {
        return nullptr;
    }
    std::string line(buf);
    std::string cmd = Joost::ToLower(Joost::GetFirstPart(line, ' '));
    int index = findCommandIndex(cmd);
    switch(index)
    {
        case -1: // No command found
            *color = 31; // Red
            *bold = 1;   // Not bold
            return strdup(" <Command not found>");
        case -2: // Multiple commands found
            *color = 31; // Red
            *bold = 1;   // Not bold
            return strdup(" <Multiple commands found>");
        default: // Single command found
            *color = 36; // Cyan
            *bold = 0;   // Not bold
            if(line.empty())
            {
                return strdup(commands[index].hint.c_str());
            }
            else
            {
                // arg hint?
            }
    }
    return nullptr;
}

void CommandsImpl::runCommand(const char *line)
{
    std::string input(line);
    std::string cmd = Joost::ToLower(Joost::GetFirstPart(input, ' '));
    int index = findCommandIndex(cmd);
    switch(index)
    {
        case -1: // No command found
            Console::write("\n\033[0;31mCommand '%s' not found\033[0m\n", cmd.c_str());
            break;
        case -2: // Multiple commands found
            Console::write("\n\033[0;31mCommand '%s' has multiple matches\033[0m\n", cmd.c_str());
            break;
        default: // Single command found
            commands[index].handler(input.c_str());
            break;
    }
}
