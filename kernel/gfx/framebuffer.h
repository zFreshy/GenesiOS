#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../include/kernel.h"
#include "../include/multiboot2.h"

typedef struct {
    uint32_t *buffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t  bpp;
} framebuffer_t;

extern framebuffer_t g_fb;

bool fb_available(void);
uint32_t fb_width(void);
uint32_t fb_height(void);

void fb_init(uint64_t mboot_info);
void fb_putpixel(uint32_t x, uint32_t y, uint32_t color);
void fb_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_clear(uint32_t color);
void fb_flip(void);
void fb_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint32_t *src);

#endif
