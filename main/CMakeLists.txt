idf_component_register(SRCS main.cpp
                            console.cpp
                            commands.cpp
                            serial.cpp
                            example_main.c
                            lvgl_demo_ui.c
                       PRIV_REQUIRES spi_flash
                                     esp_driver_uart
                                     console
                                     esp_lcd_gc9a01
                                     esp_lvgl_port
                       REQUIRES joost_onboardled
                                joost_support
                       INCLUDE_DIRS ".")
