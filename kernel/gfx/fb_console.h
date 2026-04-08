#ifndef FB_CONSOLE_H
#define FB_CONSOLE_H

#include "../include/kernel.h"

void fb_console_init(void);
void fb_console_putchar(char c);
void fb_console_puts(const char *str);
void fb_console_clear(void);

/* Colors to match VGA logic if used */
#define FBC_BLACK       0x00000000
#define FBC_WHITE       0x00FFFFFF
#define FBC_LIGHT_GREY  0x00AAAAAA
#define FBC_DARK_GREY   0x00555555
#define FBC_LIGHT_CYAN  0x0055FFFF
#define FBC_LIGHT_GREEN 0x0055FF55
#define FBC_RED         0x00FF0000

void fbc_set_fg(uint32_t color);
void fbc_set_bg(uint32_t color);
void fbc_clear(void);
void fbc_puts(const char *str);
void fbc_putchar(char c);

#endif
