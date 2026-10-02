idf_component_register(SRCS clock.cpp 
                            commands.cpp
                            console.cpp
                            main.cpp
                            serial.cpp
                       PRIV_REQUIRES console
                                     esp_driver_uart
                       REQUIRES joost_display
                                joost_onboardled
                                joost_support
                       INCLUDE_DIRS ".")
