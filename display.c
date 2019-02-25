#include <stdint.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include "display.h"

#define LOG_TAG "fairy_display"

struct display_t {
    display_config_t config;
    spi_device_handle_t device_handle;
    panel_init_func init_func;
    set_pos_func set_pos;
    fill_fb_func fill_fb;
    fill_full_fb_func fill_full_fb;
    write_fb_func write_fb;
    prep_write_fb_func prep_write_fb;
    fill_png_func fill_png;
    set_brightness_func set_brightness;
    set_power_func set_power;
};

display_handle_t display_create()
{
    display_handle_t handle = calloc(1, sizeof(struct display_t));
    if(!handle) {
        ESP_LOGE(LOG_TAG, "Failed to allocate handle memory!");
        return NULL;
    } else {
        return handle;
    }
}

void display_free(display_handle_t handle)
{
    if(handle) free(handle);
}

int display_init(display_handle_t display_handle, display_config_t *display_config)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->init_func(display_config);
    return ESP_OK;
}

int display_set_pos(display_handle_t display_handle, const uint16_t row_start, const uint16_t row_end,
                     const uint16_t col_start, const uint16_t col_end) {
    if(!display_handle) return ESP_FAIL;
    display_handle->set_pos(row_start, row_end, col_start, col_end);
    return ESP_OK;
}

int display_prep_fb(display_handle_t display_handle)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->prep_write_fb();
    return ESP_OK;
}

int display_fill_fb(display_handle_t display_handle, const uint16_t val, const uint16_t row_start,
                    const uint16_t row_end, const uint16_t col_start, const uint16_t col_end)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->fill_fb(val, row_start, row_end, col_start, col_end);
    return ESP_OK;
}

int display_fill_full_fb(display_handle_t display_handle, const uint16_t val)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->fill_full_fb(val);
    return ESP_OK;
}

int display_write_fb(display_handle_t display_handle, const uint16_t val)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->write_fb(val);
    return ESP_OK;
}

int display_fill_png(display_handle_t display_handle, const uint8_t *buf, size_t len)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->fill_png(buf, len);
    return ESP_OK;
}

int display_set_power(display_handle_t  display_handle, bool turn_on)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->set_power(turn_on);
    return ESP_OK;
}

int display_set_brightness(display_handle_t display_handle, const uint8_t brightness)
{
    if(!display_handle) return ESP_FAIL;
    display_handle->set_brightness(brightness);
    return ESP_OK;
}

void display_set_func(display_handle_t display_handle, panel_init_func init_func, set_pos_func set_pos,
                      fill_fb_func fill_fb, fill_full_fb_func fill_full_fb, write_fb_func write_fb,
                      prep_write_fb_func prep_write_fb, fill_png_func fill_png,
                      set_power_func set_power, set_brightness_func set_brightness)
{
    display_handle->init_func = init_func;
    display_handle->set_pos = set_pos;
    display_handle->fill_fb = fill_fb;
    display_handle->fill_full_fb = fill_full_fb;
    display_handle->write_fb = write_fb;
    display_handle->prep_write_fb = prep_write_fb;
    display_handle->fill_png = fill_png;
    display_handle->set_power = set_power;
    display_handle->set_brightness = set_brightness;
}
