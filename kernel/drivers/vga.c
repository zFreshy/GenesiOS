/*
 * kernel/drivers/vga.c
 * VGA 80x25 text-mode driver.
 * Physical address: 0xB8000
 * Each cell = 2 bytes: [char][attribute]
 * Attribute = (bg << 4) | fg
 */
#include "../include/vga.h"

#define VGA_BUF   ((volatile uint16_t *)0xB8000)
#define VGA_COLS  80
#define VGA_ROWS  25

/* CRT controller ports — used to move hardware cursor */
#define VGA_CTRL  0x3D4
#define VGA_DATA  0x3D5

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */
static uint8_t s_color;     /* current attribute byte */
static size_t  s_col;
static size_t  s_row;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */
static inline uint8_t make_attr(vga_color_t fg, vga_color_t bg) {
    return (uint8_t)((uint8_t)bg << 4) | (uint8_t)fg;
}

static inline uint16_t make_cell(char c, uint8_t attr) {
    return (uint16_t)(uint8_t)c | ((uint16_t)attr << 8);
}

/* Move the blinking hardware cursor to the current position */
static void update_hw_cursor(void) {
    uint16_t pos = (uint16_t)(s_row * VGA_COLS + s_col);
    outb(VGA_CTRL, 0x0F);
    outb(VGA_DATA, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL, 0x0E);
    outb(VGA_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

/* Scroll the screen up by one line */
static void scroll_up(void) {
    for (size_t y = 1; y < VGA_ROWS; y++) {
        for (size_t x = 0; x < VGA_COLS; x++) {
            VGA_BUF[(y - 1) * VGA_COLS + x] = VGA_BUF[y * VGA_COLS + x];
        }
    }
    /* Clear last row */
    for (size_t x = 0; x < VGA_COLS; x++) {
        VGA_BUF[(VGA_ROWS - 1) * VGA_COLS + x] = make_cell(' ', s_color);
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
void vga_init(void) {
    s_color = make_attr(VGA_WHITE, VGA_BLACK);
    s_col   = 0;
    s_row   = 0;
    vga_clear();
}

void vga_clear(void) {
    for (size_t i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        VGA_BUF[i] = make_cell(' ', s_color);
    }
    s_col = s_row = 0;
    update_hw_cursor();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    s_color = make_attr(fg, bg);
}

void vga_get_cursor(size_t *col, size_t *row) {
    *col = s_col;
    *row = s_row;
}

void vga_set_cursor(size_t col, size_t row) {
    s_col = col < VGA_COLS ? col : VGA_COLS - 1;
    s_row = row < VGA_ROWS ? row : VGA_ROWS - 1;
    update_hw_cursor();
}

void vga_putchar(char c) {
    switch (c) {
    case '\n':
        s_col = 0;
        if (++s_row == VGA_ROWS) { scroll_up(); s_row = VGA_ROWS - 1; }
        break;
    case '\r':
        s_col = 0;
        break;
    case '\t':
        do { vga_putchar(' '); } while (s_col % 4 != 0);
        break;
    case '\b':
        if (s_col > 0) {
            s_col--;
            VGA_BUF[s_row * VGA_COLS + s_col] = make_cell(' ', s_color);
        }
        break;
    default:
        VGA_BUF[s_row * VGA_COLS + s_col] = make_cell(c, s_color);
        if (++s_col == VGA_COLS) {
            s_col = 0;
            if (++s_row == VGA_ROWS) { scroll_up(); s_row = VGA_ROWS - 1; }
        }
        break;
    }
    update_hw_cursor();
}

void vga_puts(const char *s) {
    while (*s) vga_putchar(*s++);
}

void vga_print_u64_hex(uint64_t v) {
    static const char hex[] = "0123456789ABCDEF";
    vga_putchar('0'); vga_putchar('x');
    for (int i = 60; i >= 0; i -= 4) {
        vga_putchar(hex[(v >> i) & 0xF]);
    }
}

void vga_print_u64_dec(uint64_t v) {
    if (v == 0) { vga_putchar('0'); return; }
    char buf[21]; int len = 0;
    while (v) { buf[len++] = (char)('0' + (v % 10)); v /= 10; }
    for (int i = len - 1; i >= 0; i--) vga_putchar(buf[i]);
}
