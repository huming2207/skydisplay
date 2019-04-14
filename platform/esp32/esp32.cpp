#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp32.hpp"

void esp32::wait_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
