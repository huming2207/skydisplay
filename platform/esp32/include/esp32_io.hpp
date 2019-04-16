#pragma once

#include <map>
#include <functional>

#include <hm_io.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>


class esp32_io : hm_io
{
    public:
        esp32_io(uint8_t pin, io_pull pull, io_mode mode);
        ~esp32_io();
        hm_err_t set_pin(io_state state) override;
        io_state get_pin() override;
        hm_err_t attach_interrupt(io_intr intr_mode, std::function<void(uint8_t)> cb_func) override;


    private:
        gpio_num_t curr_pin = GPIO_NUM_MAX;
        std::function<void(uint8_t)> intr_cb_func = [](uint8_t) { ESP_LOGE("hm_io", "Nothing attached to callback"); };
        static void intr_handler(void *p);
        void intr_signal_handler();

};
