#pragma once

#include <hm.hpp>

class esp32 : hm
{
    public:
        void wait_ms(uint32_t ms) override;
};
