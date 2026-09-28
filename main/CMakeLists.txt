idf_component_register(SRCS commands.cpp
                            console.cpp
                            display.cpp
                            main.cpp
                            serial.cpp
                       PRIV_REQUIRES console
                                     esp_driver_uart
                                     esp_lcd_gc9a01
                                     spi_flash                                     
                       REQUIRES joost_onboardled
                                joost_support
                       INCLUDE_DIRS ".")
