#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

#include "serial.h"

#define CONFIG_UART_PORT_NUM UART_NUM_1
#define CONFIG_UART_BAUD_RATE (115200)
#define CONFIG_UART_RXD (GPIO_NUM_1)
#define CONFIG_UART_TXD (GPIO_NUM_0)
#define CONFIG_UART_RTS (UART_PIN_NO_CHANGE)
#define CONFIG_UART_CTS (UART_PIN_NO_CHANGE)

#define CONFIG_BUF_SIZE (1024)

class SerialImpl
{
public:
    static SerialImpl & getInstance();

    void Send(const std::string & message);
    std::string Read();
    std::string Info();
    void configure();

private:
    SerialImpl();
    SerialImpl(const SerialImpl &) = delete;
    SerialImpl(SerialImpl &&) = delete;

    uart_config_t uart_config;
};

SerialImpl::SerialImpl()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config = 
    {
        .baud_rate = CONFIG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .rx_glitch_filt_thresh = 0,
        .source_clk = UART_SCLK_APB,
        .flags = 
        { 
            .allow_pd = 0,
            .backup_before_sleep = 0
        }
    };
}

SerialImpl & SerialImpl::getInstance()
{
    static SerialImpl instance;
    return instance;
}

void SerialImpl::Send(const std::string & message)
{
    uart_write_bytes(CONFIG_UART_PORT_NUM, message.c_str(), message.size());
}
std::string SerialImpl::Read()
{
    uint8_t data[CONFIG_BUF_SIZE];

    int len = uart_read_bytes(CONFIG_UART_PORT_NUM, data, CONFIG_BUF_SIZE, pdMS_TO_TICKS(50));

    return std::string(data, data+len);
}
std::string SerialImpl::Info()
{
    return "";
}
void SerialImpl::configure()
{
    if(uart_is_driver_installed(CONFIG_UART_PORT_NUM))
    {
        uart_driver_delete(CONFIG_UART_PORT_NUM);
    }
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(CONFIG_UART_PORT_NUM, CONFIG_BUF_SIZE, CONFIG_BUF_SIZE, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CONFIG_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_UART_TXD, CONFIG_UART_RXD, CONFIG_UART_RTS, CONFIG_UART_CTS));

    // use this for casio?
    //ESP_ERROR_CHECK(uart_set_mode(CONFIG_UART_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));
}

void Serial::Send(const std::string & message)
{
    SerialImpl::getInstance().Send(message);
}

std::string Serial::Read()
{
    return SerialImpl::getInstance().Read();
}

std::string Serial::Info()
{
    return SerialImpl::getInstance().Info();
}

void Serial::configure()
{
    SerialImpl::getInstance().configure();
}