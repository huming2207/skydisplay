#pragma once

#include <vector>
#include <hm_spi.hpp>
#include <driver/spi_common.h>

class esp32_spi : hm_spi
{
    private:
        spi_host_device_t curr_host;
        spi_bus_config_t curr_bus{};
        std::vector<spi_device_handle_t> spi_device_list{};

    public:
        esp32_spi(hm_spi_bus_t &_spi_bus, spi_host_device_t host_device);
        esp32_spi(hm_spi_bus_t &_spi_bus) : esp32_spi(_spi_bus, VSPI_HOST){};
        ~esp32_spi();

        uint16_t hm_spi_add_device(hm_spi_device_t &spi_device);
        hm_err_t hm_spi_send(uint8_t *send_buf, size_t len) override;
        hm_err_t hm_spi_recv(uint8_t *send_buf, size_t send_len, uint8_t *recv_buf, size_t recv_len) override;
};

