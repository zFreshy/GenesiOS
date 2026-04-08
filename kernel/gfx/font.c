#include "font.h"
#include "framebuffer.h"
#include "../include/multiboot2.h"
#include "../include/kprintf.h"
#include "font_data.h"

void font_init(uint64_t mboot_info) {
    (void)mboot_info;
    kprintf("[FONT] Using embedded 12x24 smooth anti-aliased font data.\n");
}

void font_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    if ((unsigned char)c > 255) c = '?';

    uint32_t fg_r = (fg >> 16) & 0xFF;
    uint32_t fg_g = (fg >> 8) & 0xFF;
    uint32_t fg_b = fg & 0xFF;

    uint32_t bg_r = (bg >> 16) & 0xFF;
    uint32_t bg_g = (bg >> 8) & 0xFF;
    uint32_t bg_b = bg & 0xFF;

    for (uint32_t cy = 0; cy < FONT_HEIGHT; cy++) {
        for (uint32_t cx = 0; cx < FONT_WIDTH; cx++) {
            uint8_t alpha = font_alpha[(unsigned char)c][cy * FONT_WIDTH + cx];
            if (alpha == 255) {
                fb_putpixel(x + cx, y + cy, fg);
            } else if (alpha > 0) {
                // Alpha blend
                uint32_t r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
                uint32_t g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
                uint32_t b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;
                fb_putpixel(x + cx, y + cy, (r << 16) | (g << 8) | b);
            } else if (bg != 0x00000000) {
                fb_putpixel(x + cx, y + cy, bg);
            }
        }
    }
}

void font_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg) {
    uint32_t cur_x = x;
    while (*str) {
        font_draw_char(cur_x, y, *str, fg, bg);
        cur_x += FONT_WIDTH;
        str++;
    }
}

void font_draw_char_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    if ((unsigned char)c > 255) c = '?';

    uint32_t fg_r = (fg >> 16) & 0xFF;
    uint32_t fg_g = (fg >> 8) & 0xFF;
    uint32_t fg_b = fg & 0xFF;

    uint32_t bg_r = (bg >> 16) & 0xFF;
    uint32_t bg_g = (bg >> 8) & 0xFF;
    uint32_t bg_b = bg & 0xFF;

    for (uint32_t cy = 0; cy < FONT_HEIGHT; cy++) {
        for (uint32_t cx = 0; cx < FONT_WIDTH; cx++) {
            if (x + cx >= w || y + cy >= h) continue;
            uint8_t alpha = font_alpha[(unsigned char)c][cy * FONT_WIDTH + cx];
            if (alpha == 255) {
                buffer[(y + cy) * w + (x + cx)] = fg;
            } else if (alpha > 0) {
                uint32_t r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
                uint32_t g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
                uint32_t b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;
                buffer[(y + cy) * w + (x + cx)] = (r << 16) | (g << 8) | b;
            } else if (bg != 0x00000000) {
                buffer[(y + cy) * w + (x + cx)] = bg;
            }
        }
    }
}

void font_draw_string_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg) {
    uint32_t cur_x = x;
    while (*str) {
        font_draw_char_to_buffer(buffer, w, h, cur_x, y, *str, fg, bg);
        cur_x += FONT_WIDTH;
        str++;
    }
}

