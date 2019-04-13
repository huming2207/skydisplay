#pragma once

#include <cstdint>
#include <cstddef>
#include "hm_rets.hpp"

typedef struct __attribute__((packed))
{
    uint8_t mosi;
    uint8_t sclk;
    uint8_t miso;
    uint8_t wrpt;
    uint8_t hold;
} hm_spi_bus_t;

typedef struct __attribute__((packed))
{
    uint8_t spi_mode;
    uint8_t cs_pin;
    int32_t speed_hz;
} hm_spi_device_t;

class hm_spi
{
    private:
    public:
        virtual uint16_t hm_spi_add_device(hm_spi_device_t &spi_device) = 0;
        virtual hm_err_t hm_spi_send(uint8_t *send_buf, size_t len) = 0;
        virtual hm_err_t hm_spi_recv(uint8_t *send_buf, size_t send_len, uint8_t *recv_buf, size_t recv_len) = 0;
};

