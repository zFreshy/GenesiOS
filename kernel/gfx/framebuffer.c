#include "framebuffer.h"
#include "../include/kprintf.h"
#include "../mm/heap.h"
#include "../mm/vmm.h"

framebuffer_t g_fb = {0};
static uint32_t *s_backbuffer = NULL;

bool fb_available(void) {
    return g_fb.buffer != NULL;
}

uint32_t fb_width(void) {
    return g_fb.width;
}

uint32_t fb_height(void) {
    return g_fb.height;
}

void fb_init(uint64_t mboot_info) {
    mb2_tag_t *tag = mb2_find_tag(mboot_info, MB2_TAG_FRAMEBUF);
    if (!tag) {
        kpanic("No framebuffer tag found from Multiboot2!\n");
    }

    mb2_framebuffer_tag_t *fb_tag = (mb2_framebuffer_tag_t *)tag;

    if (fb_tag->framebuffer_bpp != 32) {
        kpanic("Framebuffer is not 32 BPP (found %u)!\n", fb_tag->framebuffer_bpp);
    }

    g_fb.buffer = (uint32_t *)(uintptr_t)fb_tag->framebuffer_addr;
    g_fb.width  = fb_tag->framebuffer_width;
    g_fb.height = fb_tag->framebuffer_height;
    g_fb.pitch  = fb_tag->framebuffer_pitch;
    g_fb.bpp    = fb_tag->framebuffer_bpp;

    uint32_t fb_size = g_fb.height * g_fb.pitch;

    /* Allocate double buffer */
    s_backbuffer = kmalloc(fb_size);
    if (!s_backbuffer) {
        kpanic("Failed to allocate backbuffer!\n");
    }
    kmemset(s_backbuffer, 0, fb_size);

    /* Also map the actual framebuffer physical memory into kernel space */
    uint64_t fb_phys = fb_tag->framebuffer_addr;
    uint64_t num_pages = ALIGN_UP(fb_size, 4096) / 4096;
    
    for (uint64_t i = 0; i < num_pages; i++) {
        /* Se o Framebuffer já estiver mapeado por ser <= 4GB no boot, isso não fará nada de mal
         * Se for > 4GB, nós mapeamos para poder acessar! Mas usando a flag PS (Huge Page) seria um problema.
         * Como o identity map atual usa Huge Pages (2MB), chamar vmm_map (que usa tabelas de 4KB)
         * em endereços do FB dentro dos 4GB vai causar um kpanic no vmm_map.
         * Solução: apenas mapear se estiver ACIMA do limite de 4GB. */
        if (fb_phys + i * 4096 >= 0x100000000ULL) {
            vmm_map(fb_phys + i * 4096, fb_phys + i * 4096, VMM_WRITABLE);
        }
    }

    kprintf("[GFX] Framebuffer %ux%ux%u at %p\n", g_fb.width, g_fb.height, g_fb.bpp, g_fb.buffer);

    extern void mouse_set_bounds(uint32_t w, uint32_t h);
    mouse_set_bounds(g_fb.width, g_fb.height);
}

void fb_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= g_fb.width || y >= g_fb.height) return;
    s_backbuffer[(y * (g_fb.pitch / 4)) + x] = color;
}

void fb_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t cy = y; cy < y + h; cy++) {
        if (cy >= g_fb.height) break;
        uint32_t *row = &s_backbuffer[cy * (g_fb.pitch / 4)];
        for (uint32_t cx = x; cx < x + w; cx++) {
            if (cx >= g_fb.width) break;
            row[cx] = color;
        }
    }
}

void fb_clear(uint32_t color) {
    uint32_t total = (g_fb.height * g_fb.pitch) / 4;
    for (uint32_t i = 0; i < total; i++) {
        s_backbuffer[i] = color;
    }
}

void fb_flip(void) {
    if (!g_fb.buffer || !s_backbuffer) return;
    kmemcpy(g_fb.buffer, s_backbuffer, g_fb.height * g_fb.pitch);
}

void fb_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint32_t *src) {
    if (!s_backbuffer || !src) return;
    for (uint32_t cy = y; cy < y + h; cy++) {
        if (cy >= g_fb.height) break;
        uint32_t *dst_row = &s_backbuffer[cy * (g_fb.pitch / 4)];
        const uint32_t *src_row = &src[(cy - y) * w];
        for (uint32_t cx = x; cx < x + w; cx++) {
            if (cx >= g_fb.width) break;
            dst_row[cx] = src_row[cx - x];
        }
    }
}
