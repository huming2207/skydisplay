#pragma once

#include <cstdint>
#include <functional>

#include "hm_rets.hpp"

typedef enum {
    IO_PULL_UP = 0,
    IO_PULL_DOWN,
    IO_PULL_UP_DOWN,
    IO_PULL_DEFAULT
} io_pull;

typedef enum {
    IO_LOW = 0,
    IO_HIGH = 1,
    IO_FLOAT = 2,
    IO_STATE_UNKNOWN,
} io_state;

typedef enum {
    IO_MODE_DISABLE = 0,
    IO_MODE_INPUT,
    IO_MODE_OUTPUT,
    IO_MODE_IN_OUT,
    IO_MODE_OD_OUTPUT,
    IO_MODE_OD_IN_OUT
} io_mode;

typedef enum {
    IO_INTR_DISABLE = 0,
    IO_INTR_RISING_EDGE = 1,
    IO_INTR_FALLING_EDGE = 2,
    IO_INTR_BOTH_EDGE = 3,
    IO_INTR_LOW_LEVEL = 4,
    IO_INTR_HIGH_LEVEL = 5
} io_intr;

class hm_io
{
    public:
        virtual hm_err_t set_pin(io_state state) = 0;
        virtual io_state get_pin() = 0;
        virtual hm_err_t attach_interrupt(io_intr intr_mode, std::function<void(uint8_t)> callback_func) = 0;

};

