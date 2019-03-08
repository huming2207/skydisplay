set(COMPONENT_SRCS "display.c" "display_ssd1351_1.5_oled.c" "display_st7789_1.3_ips.c" "lodepng/lodepng.c"
        "Display.cpp" "St7789Ips13.cpp" "platform/base/SPI.cpp" "platform/esp32/ESP32_SPI.cpp")
set(COMPONENT_ADD_INCLUDEDIRS "include" "platform/base/include" "platform/esp32/include")

register_component()
