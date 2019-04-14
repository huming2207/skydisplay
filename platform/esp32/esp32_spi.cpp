//
// Created by hu on 13/04/19.
//

#include <driver/spi_master.h>
#include <esp_log.h>
#include <cstring>
#include "esp32_spi.hpp"

#define LOG_TAG "hm_esp32_spi"

hm_err_t esp32_spi::hm_spi_send(uint8_t cs, uint8_t *send_buf, size_t len)
{
    if(!send_buf) {
        ESP_LOGE(LOG_TAG, "Payload is null!");
        return HM_ERR_NOT_FOUND;
    }

    spi_transaction_t spi_tract;
    memset(&spi_tract, 0, sizeof(spi_tract));

    spi_tract.tx_buffer = send_buf;
    spi_tract.length = len * 8;
    spi_tract.rxlength = 0;

    return spi_device_transmit(device_map[(gpio_num_t)cs], &spi_tract); // Use blocking transmit for now
}

hm_err_t esp32_spi::hm_spi_recv(uint8_t cs, uint8_t *send_buf, size_t send_len, uint8_t *recv_buf, size_t recv_len)
{
    if(!send_buf) {
        ESP_LOGE(LOG_TAG, "Payload is null!");
        return HM_ERR_NOT_FOUND;
    }

    spi_transaction_t spi_tract;
    memset(&spi_tract, 0, sizeof(spi_tract));

    spi_tract.tx_buffer = send_buf;
    spi_tract.length = send_len * 8;
    spi_tract.rx_buffer = recv_buf;
    spi_tract.rxlength = recv_len * 8;

    return spi_device_transmit(device_map[(gpio_num_t)cs], &spi_tract); // Use blocking transmit for now
}

uint8_t esp32_spi::hm_spi_add_device(hm_spi_device_t &spi_device)
{
    spi_device_interface_config_t device_config = {
            .clock_speed_hz = spi_device.speed_hz,
            .spics_io_num = spi_device.manual_cs ? -1 : spi_device.cs_pin,
            .queue_size = 7
    };

    spi_device_handle_t device_handle;

    ESP_ERROR_CHECK(spi_bus_add_device(curr_host, &device_config, &device_handle));

    // Add the device handle to the device handle list, then return its CS pin
    device_map[(gpio_num_t)spi_device.cs_pin] = device_handle;
    return spi_device.cs_pin;
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
    for(auto& device : device_map)
        ESP_ERROR_CHECK(spi_bus_remove_device(device.second));

    device_map.clear();
    ESP_ERROR_CHECK(spi_bus_free(curr_host));
}
