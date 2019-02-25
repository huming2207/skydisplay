#pragma once

#include <stdbool.h>

typedef struct {
    uint8_t dc;
    uint8_t cs;
    uint8_t cs_2;
    uint8_t cs_3;
    uint8_t cs_4;
    uint8_t miso;
    uint8_t mosi;
    uint8_t clk;
    uint8_t rst;
    uint8_t speed_mhz;
} display_config_t;

typedef struct display_t* display_handle_t;

typedef void (*panel_init_func)(display_config_t *display_config);

typedef void (*set_pos_func)(const uint16_t row_start, const uint16_t row_end,
                            const uint16_t col_start, const uint16_t col_end);

typedef void (*write_fb_func)(const uint16_t val);

typedef void (*fill_fb_func)(const uint16_t val, const uint16_t row_start,
                            const uint16_t row_end, const uint16_t col_start, const uint16_t col_end);

typedef void (*fill_full_fb_func)(const uint16_t val);

typedef void (*prep_write_fb_func)();

typedef void (*fill_png_func)(const uint8_t *buf, size_t len);

typedef void (*set_power_func)(bool turn_on);

typedef void (*set_brightness_func)(const uint8_t brightness);

display_handle_t display_create();

void display_free(display_handle_t handle);

int display_init(display_handle_t display_handle, display_config_t *display_config);

int display_set_pos(display_handle_t display_handle, uint16_t row_start, uint16_t row_end,
                    uint16_t col_start, uint16_t col_end);

int display_prep_fb(display_handle_t display_handle);

int display_fill_fb(display_handle_t display_handle, uint16_t val, uint16_t row_start,
                    uint16_t row_end, uint16_t col_start, uint16_t col_end);

int display_fill_full_fb(display_handle_t display_handle, uint16_t val);

int display_write_fb(display_handle_t display_handle, uint16_t val);

int display_set_power(display_handle_t  display_handle, bool turn_on);

int display_fill_png(display_handle_t display_handle, const uint8_t *buf, size_t len);

int display_set_brightness(display_handle_t display_handle, uint8_t brightness);

void display_set_func(display_handle_t display_handle, panel_init_func init_func, set_pos_func set_pos,
                      fill_fb_func fill_fb, fill_full_fb_func fill_full_fb, write_fb_func write_fb,
                      prep_write_fb_func prep_write_fb, fill_png_func fill_png,
                      set_power_func set_power, set_brightness_func set_brightness);