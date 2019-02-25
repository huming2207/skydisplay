#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SSD1351_CMD 0
#define SSD1351_DAT 1

#define SSD1351_WIDTH   128
#define SSD1351_HEIGHT  128

typedef struct {
    uint8_t reg;
    uint8_t data[3];
    uint8_t len;
} ssd1351_seq_t;

display_handle_t ssd1351_display_create();