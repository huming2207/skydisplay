//
// Created by hu on 13/04/19.
//

#include <esp_err.h>
#include <driver/gpio.h>
#include <thread>
#include "esp32_io.hpp"

static xQueueHandle gpio_intr_queue = NULL;

hm_err_t esp32_io::set_pin(io_state state)
{
    return gpio_set_level(curr_pin, state);
}

io_state esp32_io::get_pin()
{
    return (io_state)gpio_get_level(curr_pin);
}

esp32_io::esp32_io(uint8_t pin, io_pull pull, io_mode mode)
{
    gpio_mode_t esp_io_mode;
    curr_pin = (gpio_num_t)pin;

    switch(mode) {
        case IO_MODE_DISABLE:   esp_io_mode = GPIO_MODE_DISABLE;            break;
        case IO_MODE_INPUT:     esp_io_mode = GPIO_MODE_INPUT;              break;
        case IO_MODE_OUTPUT:    esp_io_mode = GPIO_MODE_OUTPUT;             break;
        case IO_MODE_IN_OUT:    esp_io_mode = GPIO_MODE_INPUT_OUTPUT;       break;
        case IO_MODE_OD_OUTPUT: esp_io_mode = GPIO_MODE_OUTPUT_OD;          break;
        case IO_MODE_OD_IN_OUT: esp_io_mode = GPIO_MODE_INPUT_OUTPUT_OD;    break;
        default:                esp_io_mode = GPIO_MODE_DISABLE;            break;
    }

    ESP_ERROR_CHECK(gpio_set_direction(curr_pin, esp_io_mode));

    gpio_pull_mode_t esp_pull_mode;
    switch(pull) {
        case IO_PULL_UP:        esp_pull_mode = GPIO_PULLUP_ONLY;           break;
        case IO_PULL_DOWN:      esp_pull_mode = GPIO_PULLDOWN_ONLY;         break;
        case IO_PULL_UP_DOWN:   esp_pull_mode = GPIO_PULLUP_PULLDOWN;       break;
        case IO_PULL_DEFAULT:   esp_pull_mode = GPIO_FLOATING;              break;
        default:                esp_pull_mode = GPIO_FLOATING;              break;
    }

    ESP_ERROR_CHECK(gpio_set_pull_mode(curr_pin, esp_pull_mode));
    if(gpio_intr_queue == nullptr) gpio_intr_queue = xQueueCreate(16, sizeof(uint32_t));
}

esp32_io::~esp32_io()
{
    gpio_reset_pin(curr_pin);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
hm_err_t esp32_io::attach_interrupt(io_intr intr_mode, std::function<void(uint8_t)> cb_func)
{
    esp_err_t ret = gpio_set_intr_type(curr_pin, (gpio_int_type_t)intr_mode);
    ret = ret ?: gpio_install_isr_service(0);
    ret = ret ?: gpio_isr_handler_add(curr_pin, esp32_io::intr_handler, (void*)curr_pin);
    intr_cb_func = cb_func;

    auto signal_thread = std::thread{&esp32_io::intr_signal_handler, this};
    signal_thread.join();

    return ret;
}

void esp32_io::intr_signal_handler()
{
    uint32_t io_num = 0;
    for(;;) {
        if(xQueueReceive(gpio_intr_queue, &io_num, portMAX_DELAY)) {
            intr_cb_func(io_num);
        }
    }
}
#pragma clang diagnostic pop

void esp32_io::intr_handler(void *p)
{
    xQueueSendFromISR(gpio_intr_queue, p, NULL);
}
