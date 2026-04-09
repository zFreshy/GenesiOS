#include "fb_console.h"
#include "framebuffer.h"
#include "font.h"
#include "../gui/compositor.h"

#define BG_COLOR 0x001A1A2E /* Dark blue background */
#define FG_COLOR 0x00FFFFFF /* White text */

extern int g_ui_scale;

static uint32_t s_cursor_x = 0;
static uint32_t s_cursor_y = 0;

static uint32_t s_bg_color = BG_COLOR;
static uint32_t s_fg_color = FG_COLOR;

static window_t *s_win = NULL;

void fb_console_bind_window(window_t *win) {
    s_win = win;
}

void fbc_set_fg(uint32_t color) { s_fg_color = color; }
void fbc_set_bg(uint32_t color) { s_bg_color = color; }
void fbc_clear(void) { fb_console_clear(); }
void fbc_puts(const char *str) { fb_console_puts(str); }
void fbc_putchar(char c) { fb_console_putchar(c); }

void fb_console_init(void) {
    s_cursor_x = 0;
    s_cursor_y = 0;
    if (!s_win) {
        fb_clear(s_bg_color);
        fb_flip();
    }
}

void fb_console_clear(void) {
    s_cursor_x = 0;
    s_cursor_y = 0;
    if (s_win) {
        for (uint32_t i = 0; i < s_win->width * s_win->height; i++) {
            s_win->buffer[i] = s_bg_color;
        }
        compositor_render();
    } else {
        fb_clear(s_bg_color);
        fb_flip();
    }
}

static void fb_console_scroll(void) {
    uint32_t fh = FONT_HEIGHT * g_ui_scale;
    if (s_win) {
        uint32_t row_bytes = s_win->width * fh * 4;
        uint32_t total_bytes = s_win->width * s_win->height * 4;
        kmemcpy(s_win->buffer, (uint8_t *)s_win->buffer + row_bytes, total_bytes - row_bytes);
        
        for (uint32_t y = s_win->height - fh; y < s_win->height; y++) {
            for (uint32_t x = 0; x < s_win->width; x++) {
                s_win->buffer[y * s_win->width + x] = s_bg_color;
            }
        }
        s_cursor_y -= fh;
    } else {
        uint32_t row_bytes = g_fb.pitch * fh;
        uint32_t total_bytes = g_fb.pitch * g_fb.height;
        kmemcpy(g_fb.buffer, (uint8_t *)g_fb.buffer + row_bytes, total_bytes - row_bytes);
        fb_fillrect(0, g_fb.height - fh, g_fb.width, fh, s_bg_color);
        s_cursor_y -= fh;
    }
}

void fb_console_putchar(char c) {
    uint32_t max_w = s_win ? s_win->width : g_fb.width;
    uint32_t max_h = s_win ? s_win->height : g_fb.height;
    uint32_t fw = FONT_WIDTH * g_ui_scale;
    uint32_t fh = FONT_HEIGHT * g_ui_scale;

    if (c == '\n') {
        s_cursor_x = 0;
        s_cursor_y += fh;
    } else if (c == '\r') {
        s_cursor_x = 0;
    } else if (c == '\b') {
        if (s_cursor_x >= fw) {
            s_cursor_x -= fw;
        } else if (s_cursor_y >= fh) {
            s_cursor_y -= fh;
            s_cursor_x = (max_w / fw - 1) * fw;
        }
    } else if (c == '\t') {
        s_cursor_x = (s_cursor_x + fw * 4) & ~(fw * 4 - 1);
    } else {
        if (s_cursor_x >= max_w) {
            s_cursor_x = 0;
            s_cursor_y += fh;
        }

        if (s_cursor_y + fh > max_h) {
            fb_console_scroll();
        }
        
        if (s_win) {
            font_draw_char_to_buffer_scaled(s_win->buffer, max_w, max_h, s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
        } else {
            font_draw_char_scaled(s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
        }
        s_cursor_x += fw;
    }

    if (s_cursor_x >= max_w) {
        s_cursor_x = 0;
        s_cursor_y += fh;
    }

    if (s_cursor_y + fh > max_h) {
        fb_console_scroll();
    }
    
    // Removido o compositor_render() e fb_flip() daqui!
    // Para atualizar instantaneamente na digitação única do teclado, o chamador cuidará.
    // Assim não causamos delay massivo durante strings compridas.
}

void fb_console_puts(const char *str) {
    uint32_t fw = FONT_WIDTH * g_ui_scale;
    uint32_t fh = FONT_HEIGHT * g_ui_scale;
    
    while (*str) {
        /* Desabilita renderização a cada caractere para string longa */
        char c = *str++;
        
        uint32_t max_w = s_win ? s_win->width : g_fb.width;
        uint32_t max_h = s_win ? s_win->height : g_fb.height;

        if (c == '\n') {
            s_cursor_x = 0;
            s_cursor_y += fh;
        } else if (c == '\r') {
            s_cursor_x = 0;
        } else if (c == '\b') {
            if (s_cursor_x >= fw) {
                s_cursor_x -= fw;
            } else if (s_cursor_y >= fh) {
                s_cursor_y -= fh;
                s_cursor_x = (max_w / fw - 1) * fw;
            }
        } else if (c == '\t') {
            s_cursor_x = (s_cursor_x + fw * 4) & ~(fw * 4 - 1);
        } else {
            if (s_cursor_x >= max_w) {
                s_cursor_x = 0;
                s_cursor_y += fh;
            }

            if (s_cursor_y + fh > max_h) {
                fb_console_scroll();
            }
            
            if (s_win) {
                font_draw_char_to_buffer_scaled(s_win->buffer, max_w, max_h, s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
            } else {
                font_draw_char_scaled(s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
            }
            s_cursor_x += fw;
        }

        if (s_cursor_x >= max_w) {
            s_cursor_x = 0;
            s_cursor_y += fh;
        }

        if (s_cursor_y + fh > max_h) {
            fb_console_scroll();
        }
    }
    
    /* Só atualiza a tela no final da string inteira */
    if (s_win) {
        compositor_render();
    } else {
        fb_flip();
    }
}
