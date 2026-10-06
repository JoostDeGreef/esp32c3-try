#include <stdio.h>
#include <unistd.h>
#include <array>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)
#define EXAMPLE_PIN_NUM_SCLK           4
#define EXAMPLE_PIN_NUM_MOSI           6
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
        void sleep(bool s);

    private:
        esp_lcd_panel_handle_t panel_handle = NULL;

        std::array<uint16_t,EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES> framebuffer;
        TaskHandle_t flip_task = nullptr;

        static bool onColorTransDone(esp_lcd_panel_io_handle_t panel_io,
                                     esp_lcd_panel_io_event_data_t *edata,
                                     void *user_ctx);
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

void Display::sleep(bool s)
{
    DisplayImpl::getinstance().sleep(s);
}

/***************************************************************/
/***  DisplayImpl                                            ***/
/***************************************************************/
DisplayImpl &DisplayImpl::getinstance()
{
    static DisplayImpl instance;
    return instance;
}

// this comes from adafruit.
#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

#define GC9A01A_SWRESET 0x01   ///< Software Reset (maybe, not documented)
#define GC9A01A_RDDID 0x04     ///< Read display identification information
#define GC9A01A_RDDST 0x09     ///< Read Display Status
#define GC9A01A_SLPIN 0x10     ///< Enter Sleep Mode
#define GC9A01A_SLPOUT 0x11    ///< Sleep Out
#define GC9A01A_PTLON 0x12     ///< Partial Mode ON
#define GC9A01A_NORON 0x13     ///< Normal Display Mode ON
#define GC9A01A_INVOFF 0x20    ///< Display Inversion OFF
#define GC9A01A_INVON 0x21     ///< Display Inversion ON
#define GC9A01A_DISPOFF 0x28   ///< Display OFF
#define GC9A01A_DISPON 0x29    ///< Display ON
#define GC9A01A_CASET 0x2A     ///< Column Address Set
#define GC9A01A_RASET 0x2B     ///< Row Address Set
#define GC9A01A_RAMWR 0x2C     ///< Memory Write
#define GC9A01A_PTLAR 0x30     ///< Partial Area
#define GC9A01A_VSCRDEF 0x33   ///< Vertical Scrolling Definition
#define GC9A01A_TEOFF 0x34     ///< Tearing Effect Line OFF
#define GC9A01A_TEON 0x35      ///< Tearing Effect Line ON
#define GC9A01A_MADCTL 0x36    ///< Memory Access Control
#define GC9A01A_VSCRSADD 0x37  ///< Vertical Scrolling Start Address
#define GC9A01A_IDLEOFF 0x38   ///< Idle mode OFF
#define GC9A01A_IDLEON 0x39    ///< Idle mode ON
#define GC9A01A_COLMOD 0x3A    ///< Pixel Format Set
#define GC9A01A_CONTINUE 0x3C  ///< Write Memory Continue
#define GC9A01A_TEARSET 0x44   ///< Set Tear Scanline
#define GC9A01A_GETLINE 0x45   ///< Get Scanline
#define GC9A01A_SETBRIGHT 0x51 ///< Write Display Brightness
#define GC9A01A_SETCTRL 0x53   ///< Write CTRL Display
#define GC9A01A1_POWER7 0xA7   ///< Power Control 7
#define GC9A01A_TEWC 0xBA      ///< Tearing effect width control
#define GC9A01A1_POWER1 0xC1   ///< Power Control 1
#define GC9A01A1_POWER2 0xC3   ///< Power Control 2
#define GC9A01A1_POWER3 0xC4   ///< Power Control 3
#define GC9A01A1_POWER4 0xC9   ///< Power Control 4
#define GC9A01A_RDID1 0xDA     ///< Read ID 1
#define GC9A01A_RDID2 0xDB     ///< Read ID 2
#define GC9A01A_RDID3 0xDC     ///< Read ID 3
#define GC9A01A_FRAMERATE 0xE8 ///< Frame rate control
#define GC9A01A_SPI2DATA 0xE9  ///< SPI 2DATA control
#define GC9A01A_INREGEN2 0xEF  ///< Inter register enable 2
#define GC9A01A_GAMMA1 0xF0    ///< Set gamma 1
#define GC9A01A_GAMMA2 0xF1    ///< Set gamma 2
#define GC9A01A_GAMMA3 0xF2    ///< Set gamma 3
#define GC9A01A_GAMMA4 0xF3    ///< Set gamma 4
#define GC9A01A_IFACE 0xF6     ///< Interface control
#define GC9A01A_INREGEN1 0xFE  ///< Inter register enable 1

// Color definitions
#define GC9A01A_BLACK 0x0000       ///<   0,   0,   0
#define GC9A01A_NAVY 0x000F        ///<   0,   0, 123
#define GC9A01A_DARKGREEN 0x03E0   ///<   0, 125,   0
#define GC9A01A_DARKCYAN 0x03EF    ///<   0, 125, 123
#define GC9A01A_MAROON 0x7800      ///< 123,   0,   0
#define GC9A01A_PURPLE 0x780F      ///< 123,   0, 123
#define GC9A01A_OLIVE 0x7BE0       ///< 123, 125,   0
#define GC9A01A_LIGHTGREY 0xC618   ///< 198, 195, 198
#define GC9A01A_DARKGREY 0x7BEF    ///< 123, 125, 123
#define GC9A01A_BLUE 0x001F        ///<   0,   0, 255
#define GC9A01A_GREEN 0x07E0       ///<   0, 255,   0
#define GC9A01A_CYAN 0x07FF        ///<   0, 255, 255
#define GC9A01A_RED 0xF800         ///< 255,   0,   0
#define GC9A01A_MAGENTA 0xF81F     ///< 255,   0, 255
#define GC9A01A_YELLOW 0xFFE0      ///< 255, 255,   0
#define GC9A01A_WHITE 0xFFFF       ///< 255, 255, 255
#define GC9A01A_ORANGE 0xFD20      ///< 255, 165,   0
#define GC9A01A_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define GC9A01A_PINK 0xFC18        ///< 255, 130, 198


static const gc9a01_lcd_init_cmd_t adafruit_init_cmd[] = 
{
//  {cmd, { data }, data_size, delay_ms}
    {GC9A01A_INREGEN2, (uint8_t []){},     0, 0},
    {0xEB,             (uint8_t []){0x14}, 1, 0}, // ?
    {GC9A01A_INREGEN1, (uint8_t []){},     0, 0},
    {GC9A01A_INREGEN2, (uint8_t []){},     0, 0},
    {0xEB,             (uint8_t []){0x14}, 1, 0}, // ?
    {0x84,             (uint8_t []){0x40}, 1, 0}, // ?
    {0x85,             (uint8_t []){0xFF}, 1, 0}, // ?
    {0x86,             (uint8_t []){0xFF}, 1, 0}, // ?
    {0x87,             (uint8_t []){0xFF}, 1, 0}, // ?
    {0x88,             (uint8_t []){0x0A}, 1, 0}, // ?
    {0x89,             (uint8_t []){0x21}, 1, 0}, // ?
    {0x8A,             (uint8_t []){0x00}, 1, 0}, // ?
    {0x8B,             (uint8_t []){0x80}, 1, 0}, // ?
    {0x8C,             (uint8_t []){0x01}, 1, 0}, // ?
    {0x8D,             (uint8_t []){0x01}, 1, 0}, // ?
    {0x8E,             (uint8_t []){0xFF}, 1, 0}, // ?
    {0x8F,             (uint8_t []){0xFF}, 1, 0}, // ?
    {0xB6,             (uint8_t []){0x00, 0x00}, 2, 0}, // ?
    {GC9A01A_MADCTL,   (uint8_t []){MADCTL_MX | MADCTL_BGR}, 1, 0},
    {GC9A01A_COLMOD,   (uint8_t []){0x05}, 1, 0},
    {0x90,             (uint8_t []){0x08, 0x08, 0x08, 0x08}, 4, 0}, // ?
    {0xBD,             (uint8_t []){0x06}, 1, 0}, // ?
    {0xBC,             (uint8_t []){0x00}, 1, 0}, // ?
    {0xFF,             (uint8_t []){0x60, 0x01, 0x04}, 3, 0}, // ?
    {GC9A01A1_POWER2,  (uint8_t []){0x13}, 1, 0},
    {GC9A01A1_POWER3,  (uint8_t []){0x13}, 1, 0},
    {GC9A01A1_POWER4,  (uint8_t []){0x22}, 1, 0},
    {0xBE,             (uint8_t []){0x11}, 1, 0}, // ?
    {0xE1,             (uint8_t []){0x10, 0x0E}, 2, 0}, // ?
    {0xDF,             (uint8_t []){0x21, 0x0c, 0x02}, 3, 0}, // ?
    {GC9A01A_GAMMA1,   (uint8_t []){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6, 0},
    {GC9A01A_GAMMA2,   (uint8_t []){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6, 0},
    {GC9A01A_GAMMA3,   (uint8_t []){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6, 0},
    {GC9A01A_GAMMA4,   (uint8_t []){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6, 0},
    {0xED,             (uint8_t []){0x1B, 0x0B}, 2, 0}, // ?
    {0xAE,             (uint8_t []){0x77}, 1, 0}, // ?
    {0xCD,             (uint8_t []){0x63}, 1, 0}, // ?
    // Unsure what this line (from manufacturer's boilerplate code) is
    // meant to do, but users reported issues, seems to work OK without:
//   //0x70, 9, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03, // ?
    {GC9A01A_FRAMERATE,(uint8_t []){0x34}, 1, 0},
    {0x62,             (uint8_t []){0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12, 0}, // ?
    {0x63,             (uint8_t []){0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12, 0}, // ?
    {0x64,             (uint8_t []){0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7, 0}, // ?
    {0x66,             (uint8_t []){0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10, 0}, // ?
    {0x67,             (uint8_t []){0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10, 0}, // ?
    {0x74,             (uint8_t []){0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7, 0}, // ?
    {0x98,             (uint8_t []){0x3e, 0x07}, 2, 0}, // ?
    {GC9A01A_TEON,     (uint8_t []){}, 0, 0},
    {GC9A01A_INVON,    (uint8_t []){}, 0, 0},
    {GC9A01A_SLPOUT,   (uint8_t []){}, 0, 150}, // Exit sleep
    {GC9A01A_DISPON,   (uint8_t []){}, 0, 150}, // Display on
};

static const gc9a01_vendor_config_t adafruit_init_config =
{
    adafruit_init_cmd,
    sizeof(adafruit_init_cmd) / sizeof(gc9a01_lcd_init_cmd_t)
};

void DisplayImpl::configure()
{
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = EXAMPLE_PIN_NUM_SCLK;
    buscfg.mosi_io_num = EXAMPLE_PIN_NUM_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.data4_io_num = -1;                                     
    buscfg.data5_io_num = -1;                                     
    buscfg.data6_io_num = -1;                                     
    buscfg.data7_io_num = -1;                                     
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
    io_config.on_color_trans_done = DisplayImpl::onColorTransDone;
    io_config.user_ctx = this;
    io_config.flags = {};                                                 
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST;
//    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;  // Esp setup
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR; // Adafruit in it sets it up like this
    panel_config.bits_per_pixel = 16;
    panel_config.flags = {};
    panel_config.data_endian = LCD_RGB_DATA_ENDIAN_BIG; // ignored?
    panel_config.vendor_config = (void *)&adafruit_init_config;

    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

bool DisplayImpl::onColorTransDone(esp_lcd_panel_io_handle_t panel_io,
                                    esp_lcd_panel_io_event_data_t *edata,
                                    void *user_ctx)
{
    auto *display = static_cast<DisplayImpl *>(user_ctx);

    if (display->flip_task != nullptr)
    {
        BaseType_t higher_priority_task_woken = pdFALSE;
        vTaskNotifyGiveFromISR(display->flip_task, &higher_priority_task_woken);
        return higher_priority_task_woken == pdTRUE;
    }

    return false;
}

void DisplayImpl::flip()
{
    flip_task = xTaskGetCurrentTaskHandle();

    // Make sure there is no notification left over from a previous transfer.
    ulTaskNotifyTake(pdTRUE, 0);

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(
        panel_handle,
        0, 0,
        EXAMPLE_LCD_H_RES,
        EXAMPLE_LCD_V_RES,
        framebuffer.data()));

    // The framebuffer must not be modified until the DMA transfer is complete.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    flip_task = nullptr;
}

uint16_t * DisplayImpl::getBuffer()
{
    return framebuffer.data();
}

void DisplayImpl::sleep(bool s)
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, s));
}