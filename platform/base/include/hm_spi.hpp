#pragma once

#include <cstdint>
#include <cstddef>
#include "hm_rets.hpp"

typedef struct __attribute__((packed))
{
    uint8_t mosi = 0;
    uint8_t sclk = 0;
    uint8_t miso = 0;
    uint8_t wrpt = 0;
    uint8_t hold = 0;
} hm_spi_bus_t;

typedef struct __attribute__((packed))
{
    uint8_t spi_mode = 0;
    uint8_t cs_pin = 5;
    int32_t speed_hz = 0;
    bool manual_cs = false;
} hm_spi_device_t;

class hm_spi
{
    private:
    public:
        virtual uint8_t hm_spi_add_device(hm_spi_device_t &spi_device) = 0;
        virtual hm_err_t hm_spi_send(uint8_t cs, uint8_t *send_buf, size_t len) = 0;
        virtual hm_err_t hm_spi_recv(uint8_t cs, uint8_t *send_buf, size_t send_len, uint8_t *recv_buf, size_t recv_len) = 0;
};

