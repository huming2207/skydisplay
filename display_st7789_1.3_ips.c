#include <esp_log.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <string.h>
#include <display.h>
#include <include/lodepng.h>
#include "display_st7789_1.3_ips.h"

#define LOG_TAG "st7789"

display_config_t *config;
static spi_device_handle_t device_handle;

st7789_seq_t st7789_init_seq[] = {
        {0x36, {0x00}, 1},
        {0x3A, {0x05}, 1},
        {0xB2, {0x0C, 0x0C, 0x00, 0x33, 0x33}, 5},
        {0xB7, {0x35}, 1},
        {0xBB, {0x19}, 1},
        {0xC0, {0x2C}, 1},
        {0xC2, {0x01}, 1},
        {0xC3, {0x12}, 1},
        {0xC4, {0x20}, 1},
        {0xC6, {0x0F}, 1},
        {0xD0, {0xA4, 0xA1}, 2}
};

// Fill to RE0h
uint8_t pos_gamma_val[] = {
        0xD0, 0x04, 0x0D,
        0x11, 0x13, 0x2B,
        0x3F, 0x54, 0x4C,
        0x18, 0x0D, 0x0B,
        0x1F, 0x23
};

// Fill to RE1h
uint8_t ngt_gamma_val[] = {
        0xD0, 0x04, 0x0C,
        0x11, 0x13, 0x2C,
        0x3F, 0x44, 0x51,
        0x2F, 0x1F, 0x1F,
        0x20, 0x23,
};

static void st7789_delay_ms(uint16_t time)
{
    vTaskDelay(time/portTICK_RATE_MS);
}

static void st7789_spi_send(const uint8_t *payload, size_t len, bool is_cmd)
{
    if(!payload) {
        ESP_LOGE(LOG_TAG, "Payload is null!");
        return;
    }

    spi_transaction_t spi_tract;
    memset(&spi_tract, 0, sizeof(spi_tract));

    spi_tract.tx_buffer = payload;
    spi_tract.length = len * 8;
    spi_tract.rxlength = 0;

    // ESP_LOGD(LOG_TAG, "Sending SPI payload, length : %d, is_cmd: %s", len, is_cmd ? "TRUE" : "FALSE");
    ESP_ERROR_CHECK(gpio_set_level(config->dc, is_cmd ? ST7789_CMD : ST7789_DAT));
    ESP_ERROR_CHECK(spi_device_transmit(device_handle, &spi_tract)); // Use blocking transmit for now (easier to debug)
    // ESP_LOGD(LOG_TAG, "SPI payload sent!");
}

static void st7789_send_seq(st7789_seq_t *seq)
{
    if(!seq) {
        ESP_LOGE(LOG_TAG, "Sequence is null!");
        return;
    }

    ESP_LOGD(LOG_TAG, "Writing to register 0x%02X...", seq->reg);
    st7789_spi_send(&seq->reg, 1, true);

    if(seq->len > 0) {
        st7789_spi_send(seq->data, seq->len, false);
        ESP_LOGD(LOG_TAG, "Seq data: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x; len: %u",
                 seq->data[0], seq->data[1], seq->data[2], seq->data[3], seq->data[4], seq->len);
    }
}

static void st7789_set_pos(const uint16_t row_start, const uint16_t row_end, const uint16_t col_start, const uint16_t col_end)
{
    const uint8_t col_cmd = 0x2A, row_cmd = 0x2B;
    const uint8_t col_start_buf[] = {(const uint8_t)(col_start >> 8), (const uint8_t)(col_start & 0xff)};
    const uint8_t col_end_buf[] = {(const uint8_t)(col_end >> 8), (const uint8_t)(col_end & 0xff)};
    const uint8_t row_start_buf[] = {(const uint8_t)(row_start >> 8), (const uint8_t)(row_start & 0xff)};
    const uint8_t row_end_buf[] = {(const uint8_t)(row_end >> 8), (const uint8_t)(row_end & 0xff)};

    ESP_LOGI(LOG_TAG, "Setting position in: "
                      "x1: 0x%02X, x2: 0x%02X; y1: 0x%02X, y2: 0x%02X", row_start, row_end, col_start, col_end);

    st7789_spi_send(&col_cmd, 1, true);
    st7789_spi_send(col_start_buf, 2, false);
    st7789_spi_send(col_end_buf, 2, false);

    st7789_spi_send(&row_cmd, 1, true);
    st7789_spi_send(row_start_buf, 2, false);
    st7789_spi_send(row_end_buf, 2, false);

    ESP_LOGI(LOG_TAG, "Position set!");
}

static void st7789_prep_write_fb()
{
    const uint8_t write_fb_reg = 0x2C;
    st7789_spi_send(&write_fb_reg, 1, true); // Tell the panel it's about to write something on the screen
}

static void st7789_write_fb(const uint16_t val)
{
    // Split uint16 to two uint8 to save some "context switching" times
    const uint8_t val_buf[2] = {(const uint8_t)(val >> 8), (const uint8_t)(val & 0xff)};
    st7789_spi_send(val_buf, 2, false);
}

static void st7789_fill_fb(const uint16_t val, const uint16_t row_start, const uint16_t row_end,
                    const uint16_t col_start, const uint16_t col_end)
{
    st7789_set_pos(row_start, row_end, col_start, col_end);
    st7789_prep_write_fb();

    ESP_LOGI(LOG_TAG, "Sending framebuffer with RGB value: 0x%X", val);
    for(uint8_t row = 0; row <= (row_end - row_start); row += 1) {
        for(uint8_t col = 0; col <= (col_end - col_start); col += 1) {
            st7789_write_fb(val);
        }
    }
    ESP_LOGI(LOG_TAG, "Framebuffer filled!");
}

static void st7789_fill_full_fb(const uint16_t val)
{
    st7789_fill_fb(val, 0, ST7789_IPS_WIDTH - 1, 0, ST7789_IPS_HEIGHT - 1); // send val to 128*128
}

static void st7789_init(display_config_t *_config)
{
    config = _config;

    ESP_LOGI(LOG_TAG, "Performing GPIO init...");
    ESP_ERROR_CHECK(gpio_set_direction(config->dc, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(config->rst, GPIO_MODE_OUTPUT));
    ESP_LOGI(LOG_TAG, "GPIO initialization finished, resetting OLED...");

    ESP_ERROR_CHECK(gpio_set_level(config->rst, 0));
    st7789_delay_ms(100);
    ESP_ERROR_CHECK(gpio_set_level(config->rst, 1));
    st7789_delay_ms(100);

    spi_bus_config_t bus_config = {
            .mosi_io_num = config->mosi,
            .sclk_io_num = config->clk,
            .miso_io_num = -1, // 3-Wire SPI, no MISO
            .quadhd_io_num = -1,
            .quadwp_io_num = -1
    };

    spi_device_interface_config_t device_config = {
            .clock_speed_hz = config->speed_mhz * 1000000,
            .mode = 0, // CPOL = 0, CPHA = 0???
            .spics_io_num = config->cs,
            .queue_size = 7
    };

    ESP_LOGI(LOG_TAG, "Performing SPI init...");
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &device_config, &device_handle));
    ESP_LOGI(LOG_TAG, "SPI initialization finished, sending init sequence to IPS panel...");

    // Send init sequence
    for(uint8_t curr_pos = 0; curr_pos < sizeof(st7789_init_seq)/sizeof(st7789_init_seq[0]); curr_pos += 1) {
        st7789_send_seq(&st7789_init_seq[curr_pos]);
    }

    // Throw in positive gamma to RE0h
    const uint8_t pos_gamma_reg = 0xE0;
    st7789_spi_send(&pos_gamma_reg, 1, true);
    st7789_spi_send(pos_gamma_val, sizeof(pos_gamma_val), false);

    // Throw in negative gamma to RE1h
    const uint8_t ngt_gamma_reg = 0xE1;
    st7789_spi_send(&ngt_gamma_reg, 1, true);
    st7789_spi_send(ngt_gamma_val, sizeof(ngt_gamma_val), false);

    // Display inversion on
    const uint8_t disp_inv_on_reg = 0x21;
    st7789_spi_send(&disp_inv_on_reg, 1, true);

    // Disable sleep
    const uint8_t sleep_disable_reg = 0x11;
    st7789_spi_send(&sleep_disable_reg, 1, true);

    // Turn on the panel
    const uint8_t disp_on_reg = 0x29;
    st7789_spi_send(&disp_on_reg, 1, true);

    // Fill the framebuffer (why white??)
    st7789_fill_full_fb(0xffff);
}

static void st7789_fill_png(const uint8_t *buf, size_t len)
{
    uint8_t *out_buf;
    unsigned int png_width = 0;
    unsigned int png_height = 0;

    uint32_t ret = lodepng_decode24(&out_buf, &png_width, &png_height, buf, len);
    if(ret) {
        ESP_LOGE(LOG_TAG, "Failed to decode the image, error code: %u", ret);
        if(out_buf != NULL) free(out_buf);
        return;
    }

    ESP_LOGI(LOG_TAG, "PNG decoded!");

    // Shouldn't be even bigger than the screen
    if(png_width > ST7789_IPS_WIDTH || png_height > ST7789_IPS_HEIGHT) {
        ESP_LOGE(LOG_TAG, "PNG buffer width/height is too huge: %u * %u", png_width, png_height);
        if(out_buf != NULL) free(out_buf);
        return;
    }
    ESP_LOGI(LOG_TAG, "PNG resolution: %u * %u", png_width, png_height);

    size_t png_len = png_width * png_height * 3; // 3 bytes per pixel
    uint16_t curr_pix = 0;
    uint8_t curr_red = 0, curr_grn = 0, curr_ble = 0;

    // Set position and prepare to send framebuffer
    // Width and height start from 1, so we need to -1
    st7789_set_pos(0, (const uint8_t) (png_width - 1), 0, (const uint8_t) (png_height - 1));
    st7789_prep_write_fb();

    // Skip alpha bytes - "we don't do alpha here"
    for(size_t curr_len = 0; curr_len < png_len; curr_len += 3) {
        curr_red = (out_buf[curr_len] >> 3); // Red is 5 bits
        curr_grn = (out_buf[curr_len + 1] >> 2); // Green is 6 bits
        curr_ble = (out_buf[curr_len + 2] >> 3); // Blue is 5 bits

        // 16 bit structure is: RRRRR,GGGGGG,BBBBB
        curr_pix = (curr_red << 11) | (curr_grn << 5) | curr_ble;

        // Write a pixel
        st7789_write_fb(curr_pix);
    }

    if(out_buf != NULL) free(out_buf);
}

display_handle_t st7789_display_create()
{
    display_handle_t handle = display_create();
    if(!handle) return NULL;

    display_set_func(handle, st7789_init, st7789_set_pos, st7789_fill_fb,
                     st7789_fill_full_fb, st7789_write_fb, st7789_prep_write_fb, st7789_fill_png, NULL, NULL);
    return handle;
}
