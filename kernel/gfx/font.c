#include "font.h"
#include "framebuffer.h"
#include "../include/multiboot2.h"
#include "../include/kprintf.h"
#include "font_data.h"

void font_init(uint64_t mboot_info) {
    (void)mboot_info;
    kprintf("[FONT] Using embedded 16x32 smooth anti-aliased font data.\n");
}

void font_draw_char_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    if ((unsigned char)c > 255) c = '?';

    uint32_t fg_r = (fg >> 16) & 0xFF;
    uint32_t fg_g = (fg >> 8) & 0xFF;
    uint32_t fg_b = fg & 0xFF;

    for (uint32_t cy = 0; cy < FONT_HEIGHT; cy++) {
        for (uint32_t cx = 0; cx < FONT_WIDTH; cx++) {
            if (x + cx >= w || y + cy >= h) continue;
            uint8_t alpha = font_alpha[(unsigned char)c][cy * FONT_WIDTH + cx];
            if (alpha == 255) {
                buffer[(y + cy) * w + (x + cx)] = fg;
            } else if (alpha > 0) {
                uint32_t bg_pixel;
                if (bg == 0x00000000) {
                    /* Se o bg é transparente, blend com o que já está no buffer */
                    bg_pixel = buffer[(y + cy) * w + (x + cx)];
                } else {
                    bg_pixel = bg;
                }
                
                uint32_t b_r = (bg_pixel >> 16) & 0xFF;
                uint32_t b_g = (bg_pixel >> 8) & 0xFF;
                uint32_t b_b = bg_pixel & 0xFF;

                uint32_t r = (fg_r * alpha + b_r * (255 - alpha)) / 255;
                uint32_t g = (fg_g * alpha + b_g * (255 - alpha)) / 255;
                uint32_t b = (fg_b * alpha + b_b * (255 - alpha)) / 255;
                buffer[(y + cy) * w + (x + cx)] = (r << 16) | (g << 8) | b;
            } else if (bg != 0x00000000) {
                buffer[(y + cy) * w + (x + cx)] = bg;
            }
        }
    }
}

void font_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    extern uint32_t *fb_get_backbuffer(void);
    extern uint32_t fb_pitch_words(void);
    extern uint32_t fb_height(void);
    
    uint32_t *bb = fb_get_backbuffer();
    if (bb) {
        font_draw_char_to_buffer(bb, fb_pitch_words(), fb_height(), x, y, c, fg, bg);
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

void font_draw_string_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg) {
    uint32_t cur_x = x;
    while (*str) {
        font_draw_char_to_buffer(buffer, w, h, cur_x, y, *str, fg, bg);
        cur_x += FONT_WIDTH;
        str++;
    }
}

void font_draw_char_to_buffer_scaled(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg, uint32_t scale) {
    if (scale == 1) {
        font_draw_char_to_buffer(buffer, w, h, x, y, c, fg, bg);
        return;
    }

    if ((unsigned char)c > 255) c = '?';

    uint32_t fg_r = (fg >> 16) & 0xFF;
    uint32_t fg_g = (fg >> 8) & 0xFF;
    uint32_t fg_b = fg & 0xFF;

    uint32_t scaled_w = FONT_WIDTH * scale;
    uint32_t scaled_h = FONT_HEIGHT * scale;

    for (uint32_t dy = 0; dy < scaled_h; dy++) {
        uint32_t src_y = dy / scale;
        for (uint32_t dx = 0; dx < scaled_w; dx++) {
            if (x + dx >= w || y + dy >= h) continue;

            uint32_t src_x = dx / scale;
            uint8_t alpha = font_alpha[(unsigned char)c][src_y * FONT_WIDTH + src_x];

            if (alpha == 255) {
                buffer[(y + dy) * w + (x + dx)] = fg;
            } else if (alpha > 0) {
                uint32_t bg_pixel = (bg == 0x00000000) ? buffer[(y + dy) * w + (x + dx)] : bg;
                uint32_t b_r = (bg_pixel >> 16) & 0xFF;
                uint32_t b_g = (bg_pixel >> 8) & 0xFF;
                uint32_t b_b = bg_pixel & 0xFF;

                uint32_t r = (fg_r * alpha + b_r * (255 - alpha)) / 255;
                uint32_t g = (fg_g * alpha + b_g * (255 - alpha)) / 255;
                uint32_t b = (fg_b * alpha + b_b * (255 - alpha)) / 255;
                buffer[(y + dy) * w + (x + dx)] = (r << 16) | (g << 8) | b;
            } else if (bg != 0x00000000) {
                buffer[(y + dy) * w + (x + dx)] = bg;
            }
        }
    }
}

void font_draw_char_scaled(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg, uint32_t scale) {
    extern uint32_t *fb_get_backbuffer(void);
    extern uint32_t fb_pitch_words(void);
    extern uint32_t fb_height(void);
    
    uint32_t *bb = fb_get_backbuffer();
    if (bb) {
        font_draw_char_to_buffer_scaled(bb, fb_pitch_words(), fb_height(), x, y, c, fg, bg, scale);
    }
}

void font_draw_string_scaled(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg, uint32_t scale) {
    uint32_t cur_x = x;
    while (*str) {
        font_draw_char_scaled(cur_x, y, *str, fg, bg, scale);
        cur_x += (FONT_WIDTH * scale);
        str++;
    }
}

void font_draw_string_to_buffer_scaled(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg, uint32_t scale) {
    uint32_t cur_x = x;
    while (*str) {
        font_draw_char_to_buffer_scaled(buffer, w, h, cur_x, y, *str, fg, bg, scale);
        cur_x += (FONT_WIDTH * scale);
        str++;
    }
}

