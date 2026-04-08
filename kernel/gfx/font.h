#ifndef FONT_H
#define FONT_H

#include "../include/kernel.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 8

void font_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void font_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg);

#endif
