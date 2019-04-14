#pragma once

#include <map>
#include <hm_spi.hpp>
#include <driver/spi_common.h>

class esp32_spi : hm_spi
{
    private:
        spi_host_device_t curr_host;
        spi_bus_config_t curr_bus{};
        std::map<gpio_num_t, spi_device_handle_t> device_map;

    public:
        esp32_spi(hm_spi_bus_t &_spi_bus, spi_host_device_t host_device);
        explicit esp32_spi(hm_spi_bus_t &_spi_bus) : esp32_spi(_spi_bus, VSPI_HOST){};
        ~esp32_spi();

        uint8_t hm_spi_add_device(hm_spi_device_t &spi_device) override ;
        hm_err_t hm_spi_send(uint8_t cs, uint8_t *send_buf, size_t len) override;
        hm_err_t hm_spi_recv(uint8_t cs, uint8_t *send_buf, size_t send_len, uint8_t *recv_buf, size_t recv_len) override;
};

