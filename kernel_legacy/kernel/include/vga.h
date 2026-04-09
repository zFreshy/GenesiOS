/*
 * kernel/include/vga.h
 * VGA text-mode (80x25) driver interface.
 */
#ifndef VGA_H
#define VGA_H

#include "kernel.h"

/* ------------------------------------------------------------------ */
/* VGA palette (16-color text mode)                                    */
/* ------------------------------------------------------------------ */
typedef enum {
    VGA_BLACK         = 0,
    VGA_BLUE          = 1,
    VGA_GREEN         = 2,
    VGA_CYAN          = 3,
    VGA_RED           = 4,
    VGA_MAGENTA       = 5,
    VGA_BROWN         = 6,
    VGA_LIGHT_GREY    = 7,
    VGA_DARK_GREY     = 8,
    VGA_LIGHT_BLUE    = 9,
    VGA_LIGHT_GREEN   = 10,
    VGA_LIGHT_CYAN    = 11,
    VGA_LIGHT_RED     = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW        = 14,
    VGA_WHITE         = 15,
} vga_color_t;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
void vga_init(void);
void vga_clear(void);
void vga_set_color(vga_color_t fg, vga_color_t bg);
void vga_get_cursor(size_t *col, size_t *row);
void vga_set_cursor(size_t col, size_t row);
void vga_putchar(char c);
void vga_puts(const char *s);
void vga_print_u64_hex(uint64_t value);
void vga_print_u64_dec(uint64_t value);

#endif /* VGA_H */
