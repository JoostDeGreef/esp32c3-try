idf_component_register(SRCS "joost_onboardled.cpp"
                    INCLUDE_DIRS .
                    REQUIRES esp_driver_gpio
                             joost_timer)

