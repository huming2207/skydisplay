set(COMPONENT_SRCS "display.c" "display_ssd1351_1.5_oled.c" "display_st7789_1.3_ips.c" "lodepng/lodepng.c"
        "Display.cpp" "St7789Ips13.cpp"
        "platform/base/include/hm_rets.hpp"
        "platform/esp32/esp32.cpp" "platform/esp32/esp32_spi.cpp" "platform/esp32/esp32_io.cpp"
        "platform/esp32/"
        "platform/base/include/hm.hpp" "platform/base/include/hm_io.hpp" "platform/base/include/hm_spi.hpp")
set(COMPONENT_ADD_INCLUDEDIRS "include" "platform/base/include" "platform/esp32/include")

register_component()
