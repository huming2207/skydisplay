//
// Created by hu on 13/04/19.
//

#include <driver/spi_master.h>
#include "esp32_spi.hpp"

hm_err_t esp32_spi::hm_spi_send(uint8_t *send_buf, size_t len)
{
    return 0;
}

hm_err_t esp32_spi::hm_spi_recv(uint8_t *send_buf, size_t send_len, uint8_t *recv_buf, size_t recv_len)
{
    return 0;
}

uint16_t esp32_spi::hm_spi_add_device(hm_spi_device_t &spi_device)
{
    spi_device_interface_config_t device_config = {
            .clock_speed_hz = spi_device.speed_hz,
            .spics_io_num = spi_device.cs_pin,
            .queue_size = 7
    };

    spi_device_handle_t device_handle;

    ESP_ERROR_CHECK(spi_bus_add_device(curr_host, &device_config, &device_handle));

    // Add the device handle to the device handle list, then return its index
    spi_device_list.push_back(device_handle);
    return spi_device_list.size() - 1; // TODO: This way of getting index is not thread-safe, but faster...
}

esp32_spi::esp32_spi(hm_spi_bus_t &_spi_bus, spi_host_device_t esp_spi_host) : curr_host(esp_spi_host)
{
    curr_bus.miso_io_num = _spi_bus.miso;
    curr_bus.mosi_io_num = _spi_bus.mosi;
    curr_bus.sclk_io_num = _spi_bus.sclk;
    curr_bus.quadhd_io_num = _spi_bus.hold;
    curr_bus.quadwp_io_num = _spi_bus.wrpt;

    ESP_ERROR_CHECK(spi_bus_initialize(curr_host, &curr_bus, 0));
}

esp32_spi::~esp32_spi()
{
    // Free out SPI device handle first, then free the bus
    for(auto device : spi_device_list)
        ESP_ERROR_CHECK(spi_bus_remove_device(device));

    ESP_ERROR_CHECK(spi_bus_free(curr_host));
}
