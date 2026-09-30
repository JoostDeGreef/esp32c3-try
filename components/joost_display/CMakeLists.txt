idf_component_register(
    SRCS display.cpp
         font_data.cpp
         painter.cpp
     REQUIRES esp_lcd_gc9a01
              spi_flash                                     
    INCLUDE_DIRS .)

