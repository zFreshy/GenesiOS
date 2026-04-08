/*
 * kernel/gfx/framebuffer.h
 * Linear framebuffer — drawing primitives.
 */
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../include/kernel.h"

/* Initialise from Multiboot2 framebuffer tag.  Returns 1 on success. */
int  fb_init(uint64_t mboot_info);

/* Check if framebuffer is available */
int  fb_available(void);

/* Basic dimensions */
uint32_t fb_width(void);
uint32_t fb_height(void);

/* Pixel colour helpers (pack 8-bit R,G,B → native pixel) */
static inline uint32_t fb_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

/* Draw a single pixel */
void fb_putpixel(uint32_t x, uint32_t y, uint32_t colour);

/* Fill a rectangle */
void fb_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t colour);

/* Blit an ARGB surface (src stride = w pixels) — alpha ignored */
void fb_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint32_t *src);

/* Draw a single 8×16 character at pixel position (x,y) */
void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);

/* Scroll the framebuffer up by one character row (16 px) */
void fb_scroll_up(uint32_t bg);

/* Clear the whole screen */
void fb_clear(uint32_t colour);

#endif /* FRAMEBUFFER_H */
