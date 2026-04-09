#include "fb_console.h"
#include "framebuffer.h"
#include "font.h"
#include "../gui/compositor.h"

#define BG_COLOR 0x001A1A2E /* Dark blue background */
#define FG_COLOR 0x00FFFFFF /* White text */

extern int g_ui_scale;

static uint32_t s_cursor_x = 0;
static uint32_t s_cursor_y = 0;

static window_t *s_win = NULL;

/* Padding settings (in pixels, scaled by g_ui_scale) */
#define TERM_PADDING_X 20
#define TERM_PADDING_Y 20

static uint32_t get_pad_x(void) { return s_win ? (TERM_PADDING_X * g_ui_scale) : 0; }
static uint32_t get_pad_y(void) { return s_win ? (TERM_PADDING_Y * g_ui_scale) : 0; }

static uint32_t s_bg_color = BG_COLOR;
static uint32_t s_fg_color = FG_COLOR;

void fb_console_bind_window(window_t *win) {
    s_win = win;
}

void fbc_set_fg(uint32_t color) { s_fg_color = color; }
void fbc_set_bg(uint32_t color) { s_bg_color = color; }
void fbc_clear(void) { fb_console_clear(); }
void fbc_puts(const char *str) { fb_console_puts(str); }
void fbc_putchar(char c) { fb_console_putchar(c); }

void fb_console_init(void) {
    s_cursor_x = get_pad_x();
    s_cursor_y = get_pad_y();
    if (!s_win) {
        fb_clear(s_bg_color);
        fb_flip();
    }
}

void fb_console_clear(void) {
    s_cursor_x = get_pad_x();
    s_cursor_y = get_pad_y();
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
    uint32_t pad_x = get_pad_x();
    uint32_t pad_y = get_pad_y();
    
    if (s_win) {
        /* Move content up by one font height, respecting padding */
        uint32_t row_bytes = s_win->width * fh * 4;
        uint32_t top_pad_bytes = s_win->width * pad_y * 4;
        uint32_t total_bytes = s_win->width * s_win->height * 4;
        
        /* Ensure we don't scroll padding itself */
        uint32_t copy_size = total_bytes - top_pad_bytes - row_bytes;
        
        kmemcpy(
            (uint8_t *)s_win->buffer + top_pad_bytes, 
            (uint8_t *)s_win->buffer + top_pad_bytes + row_bytes, 
            copy_size
        );
        
        /* Clear the newly exposed bottom line */
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
    uint32_t pad_x = get_pad_x();
    uint32_t pad_y = get_pad_y();
    uint32_t max_w = (s_win ? s_win->width : g_fb.width) - pad_x;
    uint32_t max_h = (s_win ? s_win->height : g_fb.height) - pad_y;
    uint32_t fw = FONT_WIDTH * g_ui_scale;
    uint32_t fh = FONT_HEIGHT * g_ui_scale;

    if (c == '\n') {
        s_cursor_x = pad_x;
        s_cursor_y += fh;
    } else if (c == '\r') {
        s_cursor_x = pad_x;
    } else if (c == '\b') {
        if (s_cursor_x >= pad_x + fw) {
            s_cursor_x -= fw;
        } else if (s_cursor_y >= pad_y + fh) {
            s_cursor_y -= fh;
            s_cursor_x = pad_x + ((max_w - pad_x) / fw - 1) * fw;
        }
    } else if (c == '\t') {
        s_cursor_x = (s_cursor_x + fw * 4) & ~(fw * 4 - 1);
        if (s_cursor_x < pad_x) s_cursor_x = pad_x;
    } else {
        if (s_cursor_x + fw > max_w) {
            s_cursor_x = pad_x;
            s_cursor_y += fh;
        }

        if (s_cursor_y + fh > max_h) {
            fb_console_scroll();
        }
        
        if (s_win) {
            font_draw_char_to_buffer_scaled(s_win->buffer, s_win->width, s_win->height, s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
        } else {
            font_draw_char_scaled(s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
        }
        s_cursor_x += fw;
    }

    if (s_cursor_x + fw > max_w) {
        s_cursor_x = pad_x;
        s_cursor_y += fh;
    }

    if (s_cursor_y + fh > max_h) {
        fb_console_scroll();
    }
}

void fb_console_puts(const char *str) {
    uint32_t pad_x = get_pad_x();
    uint32_t pad_y = get_pad_y();
    uint32_t fw = FONT_WIDTH * g_ui_scale;
    uint32_t fh = FONT_HEIGHT * g_ui_scale;
    
    while (*str) {
        char c = *str++;
        
        uint32_t max_w = (s_win ? s_win->width : g_fb.width) - pad_x;
        uint32_t max_h = (s_win ? s_win->height : g_fb.height) - pad_y;

        if (c == '\n') {
            s_cursor_x = pad_x;
            s_cursor_y += fh;
        } else if (c == '\r') {
            s_cursor_x = pad_x;
        } else if (c == '\b') {
            if (s_cursor_x >= pad_x + fw) {
                s_cursor_x -= fw;
            } else if (s_cursor_y >= pad_y + fh) {
                s_cursor_y -= fh;
                s_cursor_x = pad_x + ((max_w - pad_x) / fw - 1) * fw;
            }
        } else if (c == '\t') {
            s_cursor_x = (s_cursor_x + fw * 4) & ~(fw * 4 - 1);
            if (s_cursor_x < pad_x) s_cursor_x = pad_x;
        } else {
            if (s_cursor_x + fw > max_w) {
                s_cursor_x = pad_x;
                s_cursor_y += fh;
            }

            if (s_cursor_y + fh > max_h) {
                fb_console_scroll();
            }
            
            if (s_win) {
                font_draw_char_to_buffer_scaled(s_win->buffer, s_win->width, s_win->height, s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
            } else {
                font_draw_char_scaled(s_cursor_x, s_cursor_y, c, s_fg_color, s_bg_color, g_ui_scale);
            }
            s_cursor_x += fw;
        }

        if (s_cursor_x + fw > max_w) {
            s_cursor_x = pad_x;
            s_cursor_y += fh;
        }

        if (s_cursor_y + fh > max_h) {
            fb_console_scroll();
        }
    }
    
    /* Just mark the GUI for update, don't render synchronously here!
       Rendering here would cause O(N) full-screen redraws for a string of length N! */
    extern volatile bool g_gui_needs_update;
    g_gui_needs_update = true;
}
