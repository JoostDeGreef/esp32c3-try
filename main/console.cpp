#include <string>

#include "argtable3/argtable3.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "linenoise/linenoise.h"

#include "commands.h"
#include "console.h"
#include "sdkconfig.h"

#ifdef CONFIG_ESP_CONSOLE_USB_CDC
#error This example is incompatible with USB CDC console. Please try "console_usb" example instead.
#endif // CONFIG_ESP_CONSOLE_USB_CDC

class ConsoleImpl
{
public:
    static ConsoleImpl& getInstance();

    void write(const char* message);
    void mainLoop();

    void configurePort();
    void configureLinenoise();
    void configureCommands();

    void getCompletion(const char *buf, linenoiseCompletions *lc);
    char *getHint(const char *buf, int *color, int *bold);
private:
    ConsoleImpl();
    ConsoleImpl(const ConsoleImpl&) = delete;
    ConsoleImpl& operator=(const ConsoleImpl&) = delete;
};

void Console::write(const char* message)
{
    ConsoleImpl::getInstance().write(message);
}

void Console::mainLoop()
{
    ConsoleImpl::getInstance().mainLoop();
}

ConsoleImpl& ConsoleImpl::getInstance()
{
    static ConsoleImpl instance;
    return instance;
}

/* configure usb/jtag for serial communication */
void ConsoleImpl::configurePort()
{
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);
    /* set USB/JTAG driver configuration */
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
        .tx_buffer_size = 256,
        .rx_buffer_size = 256,
    };
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
    usb_serial_jtag_vfs_use_driver();
}

/* Configure linenoise line completion library */
void ConsoleImpl::configureLinenoise()
{
    /* Enable multiline editing. */
    linenoiseSetMultiLine(1);
    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(Commands::getCompletion); 
    linenoiseSetHintsCallback(Commands::getHint);
    /* Set command history size */
    linenoiseHistorySetMaxLen(100);
    /* Set command maximum length */
    linenoiseSetMaxLineLen(200);
    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

    int probe_status = linenoiseProbe();
    if (probe_status) 
    { 
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
    }

}

void ConsoleImpl::configureCommands()
{
    Commands::registerCommands();
}

ConsoleImpl::ConsoleImpl() 
{
    configurePort();

    configureLinenoise();    

    configureCommands();
}

void ConsoleImpl::write(const char* message)
{
    printf(message);
}

void ConsoleImpl::mainLoop()
{
    /* Main loop */
    //while (true) 
    for(int i=0;i<100;i++)
    {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(">");

        /* EOF or error */
        if (line == NULL) 
        { 
            continue;
        }
        /* if not empty*/
        if (strlen(line) > 0) 
        {
            /* Add the line to the history */
            linenoiseHistoryAdd(line);
            /* Try to run the command */
            Commands::runCommand(line);
        }

        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}

