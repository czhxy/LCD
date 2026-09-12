#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "font.h"
#include "image.h"
#define ST7789_WIDTH    240
#define ST7789_HEIGHT   240

void st7789_init(void);
void st7789_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void st7789_write_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const font_t *font);
void st7789_draw_image(uint16_t x, uint16_t y, const image_t *image);

#endif
