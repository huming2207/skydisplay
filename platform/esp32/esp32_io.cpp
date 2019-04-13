//
// Created by hu on 13/04/19.
//

#include <esp_err.h>
#include <driver/gpio.h>
#include "esp32_io.hpp"

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
}

esp32_io::~esp32_io()
{
    gpio_reset_pin(curr_pin);
}
