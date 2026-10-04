idf_component_register(SRCS clock.cpp 
                            commands.cpp
                            console.cpp
                            main.cpp
                            serial.cpp
                            wifi.cpp
                       PRIV_REQUIRES console
                                     esp_driver_uart
                                     esp_wifi
                                     esp_netif
                                     esp_event
                                     esp_system
                       REQUIRES joost_display
                                joost_onboardled
                                joost_support
                       INCLUDE_DIRS ".")
