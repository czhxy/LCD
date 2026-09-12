#ifndef __MOD_LCD_H
#define __MOD_LCD_H
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "font.h"
#include "image.h"
#include "bsp_lcd.h"

#define UI_WIDTH    240
#define UI_HEIGHT   240

#define mkcolor(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

void mod_ui_init(void);
void mod_ui_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void mod_ui_write_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const font_t *font);
void mod_ui_draw_image(uint16_t x, uint16_t y, const image_t *image);
void mod_ui_welcome_page_display(void);
void mod_ui_main_page_display(void);
#endif 
