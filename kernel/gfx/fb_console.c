/*
 * kernel/gfx/fb_console.c
 * VT100-style text console rendered on the linear framebuffer.
 * Supports multi-colour text, scrolling, backspace, and tab.
 */
#include "fb_console.h"
#include "framebuffer.h"
#include "font.h"
#include "../include/kernel.h"

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */
static uint32_t s_cols = 0;   /* characters per row */
static uint32_t s_rows = 0;   /* character rows     */
static uint32_t s_cx   = 0;   /* cursor column      */
static uint32_t s_cy   = 0;   /* cursor row         */
static uint32_t s_fg   = FBC_WHITE;
static uint32_t s_bg   = FBC_BG;

/* ------------------------------------------------------------------ */
/* fbc_init                                                            */
/* ------------------------------------------------------------------ */
void fbc_init(void) {
    s_cols = fb_width()  / FONT_WIDTH;
    s_rows = fb_height() / FONT_HEIGHT;
    s_cx   = 0;
    s_cy   = 0;
    s_fg   = FBC_WHITE;
    s_bg   = FBC_BG;
    fb_clear(s_bg);
}

/* ------------------------------------------------------------------ */
/* fbc_set_fg / fbc_set_bg                                             */
/* ------------------------------------------------------------------ */
void fbc_set_fg(uint32_t colour) { s_fg = colour; }
void fbc_set_bg(uint32_t colour) { s_bg = colour; }

/* ------------------------------------------------------------------ */
/* fbc_clear                                                           */
/* ------------------------------------------------------------------ */
void fbc_clear(void) {
    fb_clear(s_bg);
    s_cx = 0;
    s_cy = 0;
}

/* ------------------------------------------------------------------ */
/* Internal: advance cursor, scroll if needed                          */
/* ------------------------------------------------------------------ */
static void newline(void) {
    s_cx = 0;
    s_cy++;
    if (s_cy >= s_rows) {
        fb_scroll_up(s_bg);
        s_cy = s_rows - 1;
    }
}

/* ------------------------------------------------------------------ */
/* fbc_putchar                                                         */
/* ------------------------------------------------------------------ */
void fbc_putchar(char c) {
    if (c == '\n') {
        newline();
        return;
    }
    if (c == '\r') {
        s_cx = 0;
        return;
    }
    if (c == '\b') {
        if (s_cx > 0) {
            s_cx--;
            fb_draw_char(s_cx * FONT_WIDTH, s_cy * FONT_HEIGHT, ' ', s_fg, s_bg);
        }
        return;
    }
    if (c == '\t') {
        uint32_t next = (s_cx + 8) & ~7u;
        while (s_cx < next) {
            fb_draw_char(s_cx * FONT_WIDTH, s_cy * FONT_HEIGHT, ' ', s_fg, s_bg);
            s_cx++;
            if (s_cx >= s_cols) newline();
        }
        return;
    }

    fb_draw_char(s_cx * FONT_WIDTH, s_cy * FONT_HEIGHT, c, s_fg, s_bg);
    s_cx++;
    if (s_cx >= s_cols) newline();
}

/* ------------------------------------------------------------------ */
/* fbc_puts                                                            */
/* ------------------------------------------------------------------ */
void fbc_puts(const char *s) {
    if (!s) return;
    while (*s) fbc_putchar(*s++);
}
