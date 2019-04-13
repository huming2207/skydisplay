#pragma once

#include <hm_io.hpp>

class esp32_io : hm_io
{
    public:
        esp32_io(uint8_t pin, io_pull pull, io_mode mode);
        ~esp32_io();
        hm_err_t set_pin(io_state state) override;
        io_state get_pin() override;

    private:
        gpio_num_t curr_pin = GPIO_NUM_MAX;
};
