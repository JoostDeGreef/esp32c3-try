#include <stdio.h>
#include <unistd.h>
#include <array>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "display.h"

#define LCD_HOST  SPI2_HOST

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define EXAMPLE_PIN_NUM_SCLK           4
#define EXAMPLE_PIN_NUM_MOSI           6
#define EXAMPLE_PIN_NUM_MISO           5
#define EXAMPLE_PIN_NUM_LCD_DC         GPIO_NUM_7
// 8 is the onboard led, skip that
#define EXAMPLE_PIN_NUM_LCD_CS         GPIO_NUM_9
#define EXAMPLE_PIN_NUM_LCD_RST        GPIO_NUM_10
// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              240
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

using namespace std;

class DisplayImpl
{
    public:
        static DisplayImpl &getinstance();

        void configure();
        void flip();
        uint16_t * getBuffer();

    private:
        esp_lcd_panel_handle_t panel_handle = NULL;

        std::array<std::array<uint16_t,EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES>,2> framebuffer;
        uint8_t framebufferIndex = 0; 
};

/***************************************************************/
/***  Display                                                ***/
/***************************************************************/
void Display::configure()
{
    DisplayImpl::getinstance().configure();
}

void Display::flip()
{
    DisplayImpl::getinstance().flip();
}

Painter Display::getPainter()
{
    return Painter(EXAMPLE_LCD_H_RES,EXAMPLE_LCD_V_RES,DisplayImpl::getinstance().getBuffer());
}

/***************************************************************/
/***  DisplayImpl                                            ***/
/***************************************************************/
DisplayImpl &DisplayImpl::getinstance()
{
    static DisplayImpl instance;
    return instance;
}

void DisplayImpl::configure()
{
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = EXAMPLE_PIN_NUM_SCLK;
    buscfg.mosi_io_num = EXAMPLE_PIN_NUM_MOSI;
    buscfg.miso_io_num = EXAMPLE_PIN_NUM_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t);
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC;
    io_config.cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS;
    io_config.pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ;
    io_config.lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS;
    io_config.lcd_param_bits = EXAMPLE_LCD_PARAM_BITS;
    io_config.spi_mode = 0;
    io_config.trans_queue_depth = 10;
    io_config.on_color_trans_done = nullptr;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = 16;

    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

void DisplayImpl::flip()
{
    // wait for on_color_trans_done() to be done.
    // add a lock? 
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle,0,0,EXAMPLE_LCD_H_RES,EXAMPLE_LCD_V_RES,framebuffer[framebufferIndex].data()));
    framebufferIndex = 1-framebufferIndex;
}

uint16_t * DisplayImpl::getBuffer()
{
    return framebuffer[framebufferIndex].data();
}

/***************************************************************/
/***  Painter                                                ***/
/***************************************************************/

void Painter::clear()
{
    fill(RGB(0,0,0));
}

void Painter::fill(int16_t rgb)
{
    for(uint16_t i = 0; i<width*height; ++i)
    {
        buffer[i] = rgb;
    }
}

void Painter::dot(int x, int y, int16_t rgb)
{
    auto i = index(x,y);
    if(i>=0)
    {
        buffer[i] = rgb;
    }
}

void Painter::line(int x0, int y0, int x1, int y1, int16_t rgb)
{
    int dx = x1-x0;
    int dy = y1-y0;
    auto swapPointsIfNeeded = [&](bool swap)
    {
        if(swap)
        {
            std::swap(x0,x1);
            std::swap(y0,y1);
            dx = x1-x0;
            dy = y1-y0;
        }
    };
    if(abs(dx)>abs(dy))
    {
        swapPointsIfNeeded(dx<0);
        for(int i=0;i<=dx;++i)
        {
            int j = (dy*i)/dx;
            dot(x0+i,y0+j,rgb);
        }
    }
    else
    {
        swapPointsIfNeeded(dy<0);
        for(int i=0;i<=dy;++i)
        {
            int j = (dx*i)/dy;
            dot(x0+j,y0+i,rgb);
        }
    }
}

void Painter::circle(int cx, int cy, int radius, int16_t rgb)
{
    int x = radius;
    int y = 0;
    int decision = 1 - radius;

    while (x >= y)
    {
        dot(cx + x, cy + y, rgb);
        dot(cx + y, cy + x, rgb);
        dot(cx - y, cy + x, rgb);
        dot(cx - x, cy + y, rgb);
        dot(cx - x, cy - y, rgb);
        dot(cx - y, cy - x, rgb);
        dot(cx + y, cy - x, rgb);
        dot(cx + x, cy - y, rgb);

        ++y;
        if (decision <= 0)
        {
            // this follows from (y+1)^2 − y^2 = 2y + 1
            decision += 2 * y + 1;
        }
        else
        {
            --x;
            decision += 2 * (y - x) + 1;
        }
    }
}

void Painter::filled_circle(int cx, int cy, int radius, int16_t rgb)
{
    int x = radius;
    int y = 0;
    int decision = 1 - radius;

    while (x >= y)
    {
        line(cx - x, cy + y, cx + x, cy + y, rgb);
        line(cx - x, cy - y, cx + x, cy - y, rgb);
        line(cx - y, cy + x, cx + y, cy + x, rgb);
        line(cx - y, cy - x, cx + y, cy - x, rgb);

        decision += (decision <= 0) ? 2 * (++y) + 1 : 2 * ((++y) - (--x)) + 1;
    }
}

void text(int x, int y, int16_t rgb, const std::string & text)
{

}

int textWidth(int x, int y, const std::string & text)
{
    return 0;
}
