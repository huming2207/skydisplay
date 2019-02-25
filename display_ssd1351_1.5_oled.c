#include <esp_err.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include <freertos/task.h>
#include <string.h>

#include "display.h"
#include "lodepng.h"
#include "display_ssd1351_1.5_oled.h"

#define LOG_TAG "ssd1351"

static spi_device_handle_t device_handle;
display_config_t *config;

ssd1351_seq_t ssd1351_init_seq_a[] = {
        {0xFD, {0x12}, 1},
        {0xFD, {0xB1}, 1},
        {0xAE, {}, 0},
        {0xB3, {0xF1}, 1},
        {0xCA, {0x7F}, 1},
        {0xA2, {0x00}, 1},
        {0xA1, {0x00}, 1},
        {0xA0, {0x74}, 1},
        {0xB5, {0x00}, 1},
        {0xAB, {0x01}, 1},
        {0xB4, {0xA0, 0xB5, 0x55}, 3},
        {0xC1, {0xC8, 0x80, 0xC8}, 3},
        {0xC7, {0x0F}, 1}
};

// Send to RB8h
uint8_t ssd1351_gamma_lut[] = {
        0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D,
        0x0E, 0x0F, 0x10, 0x11,
        0x12, 0x13, 0x15, 0x17,
        0x19, 0x1B, 0x1D, 0x1F,
        0x21, 0x23, 0x25, 0x27,
        0x2A, 0x2D, 0x30, 0x33,
        0x36, 0x39, 0x3C, 0x3F,
        0x42, 0x45, 0x48, 0x4C,
        0x50, 0x54, 0x58, 0x5C,
        0x60, 0x64, 0x68, 0x6C,
        0x70, 0x74, 0x78, 0x7D,
        0x82, 0x87, 0x8C, 0x91,
        0x96, 0x9B, 0xA0, 0xA5,
        0xAA, 0xAF, 0xB4
};

ssd1351_seq_t ssd1351_init_seq_b[] = {
        {0xB1, {0x32}, 1},
        {0xB2, {0xA4, 0x00, 0x00}, 3},
        {0xBB, {0x17}, 1},
        {0xB6, {0x01}, 1},
        {0xBE, {0x05}, 1},
        {0xA6, {}, 0}
};

static void ssd1351_delay_ms(uint16_t ms)
{
    vTaskDelay(ms/portTICK_RATE_MS);
}

static void ssd1351_spi_send(const uint8_t *payload, size_t len, bool is_cmd)
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
    ESP_ERROR_CHECK(gpio_set_level(config->dc, is_cmd ? SSD1351_CMD : SSD1351_DAT));
    ESP_ERROR_CHECK(spi_device_transmit(device_handle, &spi_tract)); // Use blocking transmit for now (easier to debug)
    // ESP_LOGD(LOG_TAG, "SPI payload sent!");
}

static void ssd1351_send_seq(ssd1351_seq_t *seq)
{
    if(!seq) {
        ESP_LOGE(LOG_TAG, "Sequence is null!");
        return;
    }

    ESP_LOGD(LOG_TAG, "Writing to register 0x%02X...", seq->reg);
    ssd1351_spi_send(&seq->reg, 1, true);

    if(seq->len > 0) {
        ssd1351_spi_send(seq->data, seq->len, false);
        ESP_LOGD(LOG_TAG, "Seq data: 0x%02x, 0x%02x, 0x%02x; len: %u",
                seq->data[0], seq->data[1], seq->data[2], seq->len);
    }
}

static void ssd1351_set_pos(const uint16_t col_start, const uint16_t col_end, const uint16_t row_start, const uint16_t row_end)
{
    const uint8_t col_cmd = 0x15, row_cmd = 0x75;

    ESP_LOGI(LOG_TAG, "Setting position in: "
                      "x1: 0x%02X, x2: 0x%02X; y1: 0x%02X, y2: 0x%02X", row_start, row_end, col_start, col_end);

    ssd1351_spi_send(&col_cmd, 1, true);
    ssd1351_spi_send((const uint8_t *) &col_start, 1, false);
    ssd1351_spi_send((const uint8_t *) &col_end, 1, false);

    ssd1351_spi_send(&row_cmd, 1, true);
    ssd1351_spi_send((const uint8_t *) &row_start, 1, false);
    ssd1351_spi_send((const uint8_t *) &row_end, 1, false);

    ESP_LOGI(LOG_TAG, "Position set!");
}

static void ssd1351_prep_write_fb()
{
    const uint8_t write_fb_reg = 0x5c;
    ssd1351_spi_send(&write_fb_reg, 1, true); // Tell the panel it's about to write something on the screen
}

static void ssd1351_write_fb(const uint16_t val)
{
    // Split uint16 to two uint8 to save some "context switching" times
    const uint8_t val_buf[2] = {(const uint8_t)(val >> 8), (const uint8_t)(val & 0xff)};
    ssd1351_spi_send(val_buf, 2, false);
}

static void ssd1351_fill_fb(const uint16_t val, const uint16_t col_start, const uint16_t col_end,
                    const uint16_t row_start, const uint16_t row_end)
{
    ssd1351_set_pos(row_start, row_end, col_start, col_end);
    ssd1351_prep_write_fb();

    ESP_LOGI(LOG_TAG, "Sending framebuffer with RGB value: 0x%X", val);
    for(uint8_t col = 0; col <= (col_end - col_start); col += 1) {
        for(uint8_t row = 0; row <= (row_end - row_start); row += 1) {
            ssd1351_write_fb(val);
        }
    }
    ESP_LOGI(LOG_TAG, "Framebuffer filled!");
}

static void ssd1351_fill_full_fb(const uint16_t val)
{
    ssd1351_fill_fb(val, 0, 0x7f, 0, 0x7f); // send val to 128*128
}

static void ssd1351_set_power(bool turn_on)
{
    const uint8_t pwr_reg = turn_on ? (const uint8_t)0xAF : (const uint8_t)0xAE;
    ESP_LOGI(LOG_TAG, "Setting panel power to %s, reg: 0x%02X", turn_on ? "on" : "off", pwr_reg);
    ssd1351_spi_send(&pwr_reg, 1, true);
}

static void ssd1351_init(display_config_t *_config)
{
    config = _config;
    ESP_LOGI(LOG_TAG, "Performing GPIO init...");
    ESP_ERROR_CHECK(gpio_set_direction(config->dc, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(config->rst, GPIO_MODE_OUTPUT));
    ESP_LOGI(LOG_TAG, "GPIO initialization finished, resetting OLED...");

    ESP_ERROR_CHECK(gpio_set_level(config->rst, 1));
    ssd1351_delay_ms(100);
    ESP_ERROR_CHECK(gpio_set_level(config->rst, 0));
    ssd1351_delay_ms(100); // TODO: check pp.29: 2us is enough for reset triggering??
    ESP_ERROR_CHECK(gpio_set_level(config->rst, 1));
    ssd1351_delay_ms(100);


    spi_bus_config_t bus_config = {
        .mosi_io_num = config->mosi,
        .sclk_io_num = config->clk,
        .miso_io_num = -1, // 3-Wire SPI, no MISO
        .quadhd_io_num = -1,
        .quadwp_io_num = -1
    };

    spi_device_interface_config_t device_config = {
            .clock_speed_hz = config->speed_mhz * 1000000,
            .mode = 3, // CPOL = 1, CPHA = 1???
            .spics_io_num = config->cs,
            .queue_size = 7,
            .flags = (SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX)
    };

    ESP_LOGI(LOG_TAG, "Performing SPI init...");
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, 0));
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &device_config, &device_handle));
    ESP_LOGI(LOG_TAG, "SPI initialization finished, sending init sequence to OLED...");

    // Send initialisation sequence Group A
    for(int seq_index = 0; seq_index < sizeof(ssd1351_init_seq_a)/sizeof(ssd1351_init_seq_a[0]); seq_index += 1) {
        ssd1351_send_seq(&ssd1351_init_seq_a[seq_index]);
    }

    // Send Gamma LUT
    const uint8_t gamma_lut_reg = 0xB8;
    ssd1351_spi_send(&gamma_lut_reg, 1, true);
    ssd1351_spi_send(ssd1351_gamma_lut, sizeof(ssd1351_gamma_lut), false);

    // Send initialisation sequence Group B
    for(int seq_index = 0; seq_index < sizeof(ssd1351_init_seq_b)/sizeof(ssd1351_init_seq_b[0]); seq_index += 1) {
        ssd1351_send_seq(&ssd1351_init_seq_b[seq_index]);
    }

    // Clear display
    ssd1351_fill_full_fb(0x0000);

    // Turn the panel on
    ssd1351_set_power(true);
}

static void ssd1351_set_brightness(const uint8_t level)
{
    ESP_LOGI(LOG_TAG, "Setting brightness to: %u...", level);
    const uint8_t reg = 0xC7;
    ssd1351_spi_send(&reg, 1, true);
    ssd1351_spi_send(&level, 1, false);
}

static void ssd1351_fill_png(const uint8_t *buf, size_t len)
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
    if(png_width > SSD1351_WIDTH || png_height > SSD1351_HEIGHT) {
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
    ssd1351_set_pos(0, (const uint8_t) (png_width - 1), 0, (const uint8_t) (png_height - 1));
    ssd1351_prep_write_fb();

    // Skip alpha bytes - "we don't do alpha here"
    for(size_t curr_len = 0; curr_len < png_len; curr_len += 3) {
        curr_red = (out_buf[curr_len] >> 3); // Red is 5 bits
        curr_grn = (out_buf[curr_len + 1] >> 2); // Green is 6 bits
        curr_ble = (out_buf[curr_len + 2] >> 3); // Blue is 5 bits

        // 16 bit structure is: RRRRR,GGGGGG,BBBBB
        curr_pix = (curr_red << 11) | (curr_grn << 5) | curr_ble;

        // Write a pixel
        ssd1351_write_fb(curr_pix);
    }

    if(out_buf != NULL) free(out_buf);
}

display_handle_t ssd1351_display_create()
{
    display_handle_t handle = display_create();
    if(!handle) return NULL;

    display_set_func(handle, ssd1351_init, ssd1351_set_pos, ssd1351_fill_fb,
            ssd1351_fill_full_fb, ssd1351_write_fb, ssd1351_prep_write_fb,
            ssd1351_fill_png, ssd1351_set_power, ssd1351_set_brightness);

    return handle;
}