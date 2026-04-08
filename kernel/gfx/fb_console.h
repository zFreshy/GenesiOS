/*
 * kernel/gfx/fb_console.h
 * Text console rendered on the linear framebuffer.
 */
#ifndef FB_CONSOLE_H
#define FB_CONSOLE_H

#include "../include/kernel.h"

/* Colour palette (ARGB 0x00RRGGBB) */
#define FBC_BLACK        0x00000000U
#define FBC_DARK_GREY    0x00444444U
#define FBC_GREY         0x00AAAAAAAU
#define FBC_WHITE        0x00CCCCCCU
#define FBC_CYAN         0x0000CCCCU
#define FBC_LIGHT_CYAN   0x0055FFFFU
#define FBC_GREEN        0x0000AA00U
#define FBC_LIGHT_GREEN  0x0055FF55U
#define FBC_YELLOW       0x00FFFF55U
#define FBC_RED          0x00AA0000U
#define FBC_LIGHT_RED    0x00FF5555U
#define FBC_BLUE         0x000000AAU
#define FBC_MAGENTA      0x00AA00AAU
#define FBC_BG           0x0012121FU  /* dark navy background */

/* Initialise the framebuffer console */
void fbc_init(void);

/* Output one character (handles \n, \b, \t) */
void fbc_putchar(char c);

/* Output a null-terminated string */
void fbc_puts(const char *s);

/* Set text / background colour */
void fbc_set_fg(uint32_t colour);
void fbc_set_bg(uint32_t colour);

/* Clear the screen */
void fbc_clear(void);

#endif /* FB_CONSOLE_H */
