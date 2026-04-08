#include "fb_console.h"
#include "framebuffer.h"
#include "font.h"

#define BG_COLOR 0x001A1A2E /* Dark blue background */
#define FG_COLOR 0x00FFFFFF /* White text */

static uint32_t s_cursor_x = 0;
static uint32_t s_cursor_y = 0;

static uint32_t s_bg_color = BG_COLOR;
static uint32_t s_fg_color = FG_COLOR;

void fbc_set_fg(uint32_t color) { s_fg_color = color; }
void fbc_set_bg(uint32_t color) { s_bg_color = color; }
void fbc_clear(void) { fb_console_clear(); }
void fbc_puts(const char *str) { fb_console_puts(str); }
void fbc_putchar(char c) { fb_console_putchar(c); }

void fb_console_init(void) {
    s_cursor_x = 0;
    s_cursor_y = 0;
    fb_clear(s_bg_color);
    fb_flip();
}

void fb_console_clear(void) {
    s_cursor_x = 0;
    s_cursor_y = 0;
    fb_clear(s_bg_color);
    fb_flip();
}

static void fb_console_scroll(void) {
    /* Copy everything up by one row of text */
    uint32_t row_bytes = g_fb.pitch * FONT_HEIGHT;
    uint32_t total_bytes = g_fb.pitch * g_fb.height;
    
    kmemcpy(g_fb.buffer, (uint8_t *)g_fb.buffer + row_bytes, total_bytes - row_bytes);
    
    /* Clear the last line */
    fb_fillrect(0, g_fb.height - FONT_HEIGHT, g_fb.width, FONT_HEIGHT, s_bg_color);
    s_cursor_y -= FONT_HEIGHT;
}

void fb_console_putchar(char c) {
    if (c == '\n') {
        s_cursor_x = 0;
        s_cursor_y += FONT_HEIGHT;
    } else if (c == '\r') {
        s_cursor_x = 0;
    } else if (c == '\b') {
        if (s_cursor_x >= FONT_WIDTH) {
            s_cursor_x -= FONT_WIDTH;
        } else if (s_cursor_y >= FONT_HEIGHT) {
            s_cursor_y -= FONT_HEIGHT;
            s_cursor_x = (g_fb.width / FONT_WIDTH - 1) * FONT_WIDTH;
        }
    } else if (c == '\t') {
        s_cursor_x = (s_cursor_x + FONT_WIDTH * 4) & ~(FONT_WIDTH * 4 - 1);
    } else {
        font_draw_char(s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color);
        s_cursor_x += FONT_WIDTH;
    }

    if (s_cursor_x >= g_fb.width) {
        s_cursor_x = 0;
        s_cursor_y += FONT_HEIGHT;
    }

    if (s_cursor_y >= g_fb.height) {
        fb_console_scroll();
    }
    
    fb_flip();
}

void fb_console_puts(const char *str) {
    while (*str) {
        fb_console_putchar(*str++);
    }
    fb_flip();
}
