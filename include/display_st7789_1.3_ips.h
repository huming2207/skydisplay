#pragma once

#include <stdint.h>
#include "display.h"

#define ST7789_CMD 0
#define ST7789_DAT 1

#define ST7789_IPS_WIDTH    240
#define ST7789_IPS_HEIGHT   240

typedef struct {
    uint8_t reg;
    uint8_t data[6];
    uint8_t len;
} st7789_seq_t;

display_handle_t st7789_display_create();

