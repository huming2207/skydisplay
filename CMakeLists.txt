set(COMPONENT_SRCS "display.c" "display_ssd1351_1.5_oled.c" "display_st7789_1.3_ips.c" "lodepng/lodepng.c"
        "Display.cpp" "St7789Ips13.cpp" "platform/base/Hal.cpp" "platform/esp32/ESP32.cpp")
set(COMPONENT_ADD_INCLUDEDIRS "include" "platform/base/include" "platform/esp32/include")

register_component()
