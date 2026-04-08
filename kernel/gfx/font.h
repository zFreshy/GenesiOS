#ifndef FONT_H
#define FONT_H

#include "../include/kernel.h"
#include "font_data.h"

void font_init(uint64_t mboot_info);
void font_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void font_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg);
void font_draw_char_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void font_draw_string_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg);

void font_draw_char_scaled(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg, uint32_t scale);
void font_draw_string_scaled(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg, uint32_t scale);
void font_draw_char_to_buffer_scaled(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg, uint32_t scale);
void font_draw_string_to_buffer_scaled(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg, uint32_t scale);

#endif
