#include <esp_err.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include <freertos/task.h>
#include <cstring>

#include "ESP32.h"

#define LOG_TAG "skydisp_hal_esp32"

ESP32::ESP32(HalIO &_io)
{
    spi_bus_config_t busConfig = {
            .miso_io_num = _io.miso,
            .mosi_io_num = _io.mosi,
            .sclk_io_num = _io.sclk,
            .quadhd_io_num = -1,
            .quadwp_io_num = -1
    };

    spi_device_interface_config_t deviceConfig = {
            .clock_speed_hz = _io.spiSpeedMHz * 1000000,
            .spics_io_num = -1, // Not using auto CS control
            .queue_size = 7
    };

    deviceConfig.flags = (_io.miso == -1) ? (SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX) : 0;

    ESP_LOGI(LOG_TAG, "Performing SPI init...");

    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &busConfig, 0));
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &deviceConfig, &spiDeviceHandle));

    ESP_LOGI(LOG_TAG, "SPI initialization finished.");

}

ESP32::~ESP32()
{
    spi_bus_free(VSPI_HOST);
}

void ESP32::WriteSPI(uint8_t *payload, size_t len, uint8_t dcLvl)
{
    if(!payload) {
        ESP_LOGE(LOG_TAG, "Payload is null!");
        return;
    }

    spi_transaction_t spiTransaction;
    memset(&spiTransaction, 0, sizeof(spiTransaction));

    spiTransaction.tx_buffer = payload;
    spiTransaction.length = len * 8;
    spiTransaction.rxlength = 0;

    // ESP_LOGD(LOG_TAG, "Sending SPI payload, length : %d, is_cmd: %s", len, is_cmd ? "TRUE" : "FALSE");
    ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)io.dc, dcLvl ? 0 : 1));
    ESP_ERROR_CHECK(spi_device_transmit(spiDeviceHandle, &spiTransaction)); // Use blocking transmit for now
    // ESP_LOGD(LOG_TAG, "SPI payload sent!");
}

void ESP32::ReadSPI(uint8_t *recvBuf, size_t len, uint8_t dcLvl)
{

}

void ESP32::WaitSPI(uint32_t waitMS)
{
    vTaskDelay(pdMS_TO_TICKS(waitMS));
}

void ESP32::ToggleIO(int8_t pin, bool on)
{
    ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)pin, on ? 1 : 0));
}

void ESP32::PrepareIO(int8_t pin, bool isOutput)
{
    ESP_ERROR_CHECK(gpio_set_direction((gpio_num_t)pin, isOutput ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT));
}

bool ESP32::ReadIO(int8_t pin)
{
    return gpio_get_level((gpio_num_t)pin) == 1;
}
