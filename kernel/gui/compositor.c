/*
 * kernel/gui/compositor.c
 * Renders all windows and the mouse cursor to a backbuffer, then blits.
 */
#include "compositor.h"
#include "window.h"
#include "../gfx/framebuffer.h"
#include "../drivers/mouse.h"
#include "../mm/heap.h"
#include "../include/kprintf.h"

static uint32_t *s_backbuffer = NULL;
static uint32_t  s_width      = 0;
static uint32_t  s_height     = 0;

/* ------------------------------------------------------------------ */
/* Initialize compositor                                              */
/* ------------------------------------------------------------------ */
void compositor_init(void) {
    if (!fb_available()) return;

    s_width  = fb_width();
    s_height = fb_height();

    s_backbuffer = (uint32_t *)kmalloc(s_width * s_height * sizeof(uint32_t));
    if (!s_backbuffer) {
        kprintf("[COMP] Failed to allocate backbuffer\n");
    }
}

/* ------------------------------------------------------------------ */
/* Copy window pixels to backbuffer (handling clipping)               */
/* ------------------------------------------------------------------ */
static void draw_window(window_t *win) {
    if (!s_backbuffer || !win || !win->buffer) return;

    /* Calculate bounds and clip to screen */
    int32_t start_x = win->x < 0 ? 0 : win->x;
    int32_t start_y = win->y < 0 ? 0 : win->y;
    
    int32_t end_x   = win->x + (int32_t)win->width;
    int32_t end_y   = win->y + (int32_t)win->height;
    
    if (end_x > (int32_t)s_width)  end_x = (int32_t)s_width;
    if (end_y > (int32_t)s_height) end_y = (int32_t)s_height;

    for (int32_t sy = start_y; sy < end_y; sy++) {
        uint32_t *dst_row = s_backbuffer + (sy * s_width);
        uint32_t *src_row = win->buffer + ((sy - win->y) * win->width);
        
        for (int32_t sx = start_x; sx < end_x; sx++) {
            dst_row[sx] = src_row[sx - win->x];
        }
    }
    
    /* Draw window title bar (temporary simple decorator) */
    if (win->y >= 16) {
        int32_t title_y = win->y - 16;
        for (int sy = title_y; sy < win->y; sy++) {
            if (sy < 0 || sy >= (int32_t)s_height) continue;
            uint32_t *dst_row = s_backbuffer + (sy * s_width);
            for (int sx = start_x; sx < end_x; sx++) {
                dst_row[sx] = 0x004488FF; /* Blue title bar */
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw a cursor cross onto the backbuffer                            */
/* ------------------------------------------------------------------ */
static void draw_cursor(void) {
    if (!s_backbuffer) return;
    int32_t cx = mouse_x();
    int32_t cy = mouse_y();
    uint32_t cursor_color = 0x00FFFFFF; /* White cursor */

    for (int i = -5; i <= 5; i++) {
        int32_t py = cy + i;
        int32_t px = cx + i;
        if (py >= 0 && py < (int32_t)s_height && cx >= 0 && cx < (int32_t)s_width) {
            s_backbuffer[py * s_width + cx] = cursor_color;
        }
        if (px >= 0 && px < (int32_t)s_width && cy >= 0 && cy < (int32_t)s_height) {
            s_backbuffer[cy * s_width + px] = cursor_color;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Render all layers to screen                                        */
/* ------------------------------------------------------------------ */
void compositor_render(void) {
    if (!s_backbuffer || !fb_available()) return;

    /* 1. Clear background (Dark theme) */
    for (uint32_t i = 0; i < s_width * s_height; i++) {
        s_backbuffer[i] = 0x00222222;
    }

    /* 2. Draw windows back-to-front */
    window_t *win = wm_get_bottom();
    while (win) {
        draw_window(win);
        win = win->next;
    }

    /* 3. Draw mouse cursor overlay */
    draw_cursor();

    /* 4. Blit backbuffer entirely to screen */
    fb_blit(0, 0, s_width, s_height, s_backbuffer);
}
