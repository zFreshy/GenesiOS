/*
 * kernel/gfx/font.h
 * Bitmap font renderer — 8x16 embedded font.
 */
#ifndef FONT_H
#define FONT_H

#include "../include/kernel.h"

#define FONT_WIDTH   8
#define FONT_HEIGHT  16

/* Pointer to the raw bitmap data: 256 chars × 16 bytes */
extern const uint8_t g_font8x16[256][16];

#endif /* FONT_H */
