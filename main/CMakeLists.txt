idf_component_register(SRCS main.cpp
                            console.cpp
                            commands.cpp
                            serial.cpp
                       PRIV_REQUIRES spi_flash
                                     esp_driver_uart
                                     console
                                     gc9a01
                       REQUIRES joost_onboardled
                                joost_support
                       INCLUDE_DIRS ".")
