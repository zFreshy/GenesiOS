/*
 * kernel/gfx/framebuffer.c
 * Linear framebuffer driver — parses Multiboot2 tag and exposes pixel ops.
 */
#include "framebuffer.h"
#include "font.h"
#include "../mm/vmm.h"
#include "../include/multiboot2.h"
#include "../include/kprintf.h"

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */
static uint32_t *s_fb     = NULL;   /* pointer to framebuffer memory  */
static uint32_t  s_width  = 0;
static uint32_t  s_height = 0;
static uint32_t  s_pitch  = 0;     /* bytes per scanline              */
static uint32_t  s_bpp    = 0;

/* ------------------------------------------------------------------ */
/* fb_init                                                             */
/* ------------------------------------------------------------------ */
int fb_init(uint64_t mboot_info) {
    mb2_framebuffer_tag_t *tag =
        (mb2_framebuffer_tag_t *)mb2_find_tag(mboot_info, MB2_TAG_FRAMEBUF);

    if (!tag) {
        kprintf("[FB] No framebuffer tag from GRUB — staying in VGA text mode\n");
        return 0;
    }
    if (tag->framebuffer_type != MB2_FB_TYPE_RGB) {
        kprintf("[FB] Framebuffer not in RGB mode (type=%u)\n", tag->framebuffer_type);
        return 0;
    }
    if (tag->framebuffer_bpp != 32) {
        kprintf("[FB] Need 32 bpp, got %u\n", tag->framebuffer_bpp);
        return 0;
    }

    s_fb     = (uint32_t *)(uintptr_t)tag->framebuffer_addr;
    s_width  = tag->framebuffer_width;
    s_height = tag->framebuffer_height;
    s_pitch  = tag->framebuffer_pitch;
    s_bpp    = tag->framebuffer_bpp;

    /* Map the framebuffer (identity map) since it is likely above 256MB */
    uint32_t fb_size_bytes = s_pitch * s_height;
    for (uint32_t offset = 0; offset < fb_size_bytes; offset += 4096) {
        vmm_map(tag->framebuffer_addr + offset, 
                tag->framebuffer_addr + offset, 
                VMM_PRESENT | VMM_WRITABLE);
    }

    kprintf("[FB] %ux%u @32bpp  addr=0x%llx pitch=%u mapped %u pages\n",
            s_width, s_height,
            (unsigned long long)tag->framebuffer_addr,
            s_pitch,
            (fb_size_bytes + 4095) / 4096);
    return 1;
}

int      fb_available(void) { return s_fb != NULL; }
uint32_t fb_width(void)     { return s_width;  }
uint32_t fb_height(void)    { return s_height; }

/* ------------------------------------------------------------------ */
/* fb_putpixel                                                         */
/* ------------------------------------------------------------------ */
void fb_putpixel(uint32_t x, uint32_t y, uint32_t colour) {
    if (x >= s_width || y >= s_height) return;
    uint32_t *row = (uint32_t *)((uint8_t *)s_fb + y * s_pitch);
    row[x] = colour;
}

/* ------------------------------------------------------------------ */
/* fb_fillrect                                                         */
/* ------------------------------------------------------------------ */
void fb_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t colour) {
    if (!s_fb) return;
    if (x >= s_width || y >= s_height) return;
    if (x + w > s_width)  w = s_width  - x;
    if (y + h > s_height) h = s_height - y;

    for (uint32_t row = 0; row < h; row++) {
        uint32_t *line = (uint32_t *)((uint8_t *)s_fb + (y + row) * s_pitch) + x;
        for (uint32_t col = 0; col < w; col++)
            line[col] = colour;
    }
}

/* ------------------------------------------------------------------ */
/* fb_blit                                                             */
/* ------------------------------------------------------------------ */
void fb_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint32_t *src) {
    if (!s_fb) return;
    for (uint32_t row = 0; row < h && (y + row) < s_height; row++) {
        uint32_t *dst_row = (uint32_t *)((uint8_t *)s_fb + (y + row) * s_pitch) + x;
        const uint32_t *src_row = src + row * w;
        for (uint32_t col = 0; col < w && (x + col) < s_width; col++)
            dst_row[col] = src_row[col];
    }
}

/* ------------------------------------------------------------------ */
/* fb_draw_char — render one glyph from the embedded 8×16 font        */
/* ------------------------------------------------------------------ */
void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    if (!s_fb) return;
    uint8_t idx = (uint8_t)c;
    for (uint32_t row = 0; row < FONT_HEIGHT; row++) {
        uint8_t bits = g_font8x16[idx][row];
        uint32_t *line = (uint32_t *)((uint8_t *)s_fb + (y + row) * s_pitch) + x;
        for (int col = 0; col < FONT_WIDTH; col++) {
            line[col] = (bits & (0x80 >> col)) ? fg : bg;
        }
    }
}

/* ------------------------------------------------------------------ */
/* fb_scroll_up — scroll the screen up by one text row (16 px)        */
/* ------------------------------------------------------------------ */
void fb_scroll_up(uint32_t bg) {
    if (!s_fb) return;

    /* Move all rows up by FONT_HEIGHT pixels */
    uint32_t move_bytes = s_pitch * (s_height - FONT_HEIGHT);
    uint8_t *dst = (uint8_t *)s_fb;
    uint8_t *src = (uint8_t *)s_fb + s_pitch * FONT_HEIGHT;
    for (uint32_t i = 0; i < move_bytes; i++) dst[i] = src[i];

    /* Clear the bottom row */
    fb_fillrect(0, s_height - FONT_HEIGHT, s_width, FONT_HEIGHT, bg);
}

/* ------------------------------------------------------------------ */
/* fb_clear                                                            */
/* ------------------------------------------------------------------ */
void fb_clear(uint32_t colour) {
    fb_fillrect(0, 0, s_width, s_height, colour);
}
