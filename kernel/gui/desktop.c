/*
 * kernel/gui/desktop.c
 * Basic OS desktop layout and window creation.
 */
#include "desktop.h"
#include "window.h"
#include "compositor.h"
#include "../gfx/framebuffer.h"
#include "../mm/heap.h"

static window_t *s_win1 = NULL;
static window_t *s_win2 = NULL;

#include "../gfx/font.h"

static void draw_text_to_buffer(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg) {
    uint32_t cur_x = x;
    while (*str) {
        char c = *str++;
        /* Simplistic font rendering directly to a memory buffer */
        /* Copying logic from font.c but pointing to our buffer */
        extern const uint8_t font8x8[128][8];
        for (int cy = 0; cy < 8; cy++) {
            uint8_t row = font8x8[(int)c][cy];
            for (int cx = 0; cx < 8; cx++) {
                if (cur_x + cx >= w || y + cy >= h) continue;
                if (row & (1 << (7 - cx))) {
                    buffer[(y + cy) * w + (cur_x + cx)] = fg;
                } else if (bg != 0) {
                    buffer[(y + cy) * w + (cur_x + cx)] = bg;
                }
            }
        }
        cur_x += 8;
    }
}

void desktop_create_terminal(void) {
    window_t *win = wm_create_window(100, 100, 300, 200, "Terminal");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 300 * 200; i++) {
            win->buffer[i] = 0x001E1E1E; /* Dark grey */
        }
        draw_text_to_buffer(win->buffer, 300, 200, 8, 8, "genesi> run test_app", 0x00FFFFFF, 0x001E1E1E);
        draw_text_to_buffer(win->buffer, 300, 200, 8, 20, "Executing...", 0x00AAAAAA, 0x001E1E1E);
        draw_text_to_buffer(win->buffer, 300, 200, 8, 32, "Hello from Ring 3!", 0x0055FF55, 0x001E1E1E);
    }
}

void desktop_create_explorer(void) {
    window_t *win = wm_create_window(250, 150, 400, 300, "File Explorer");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 400 * 300; i++) {
            win->buffer[i] = 0x00EEEEEE; /* Light grey/white */
        }
        for (uint32_t y = 0; y < 24; y++) {
            for (uint32_t x = 0; x < 400; x++) {
                win->buffer[y * 400 + x] = 0x00DDDDDD;
            }
        }
        draw_text_to_buffer(win->buffer, 400, 300, 8, 8, "C:\\ >", 0x00000000, 0x00DDDDDD);
        draw_text_to_buffer(win->buffer, 400, 300, 16, 40, "[+] kernel", 0x00000000, 0x00EEEEEE);
        draw_text_to_buffer(win->buffer, 400, 300, 16, 56, "[+] user", 0x00000000, 0x00EEEEEE);
        draw_text_to_buffer(win->buffer, 400, 300, 16, 72, "    hello.txt", 0x00000000, 0x00EEEEEE);
        draw_text_to_buffer(win->buffer, 400, 300, 16, 88, "    test_app.elf", 0x00000000, 0x00EEEEEE);
    }
}

void desktop_create_sysinfo(void) {
    window_t *win = wm_create_window(400, 200, 250, 150, "System Info");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 250 * 150; i++) {
            win->buffer[i] = 0x00111155; /* Blue */
        }
        draw_text_to_buffer(win->buffer, 250, 150, 8, 8, "Genesi OS v0.2", 0x00FFFFFF, 0x00111155);
        draw_text_to_buffer(win->buffer, 250, 150, 8, 32, "Memory: 512 MB", 0x00FFFFFF, 0x00111155);
        draw_text_to_buffer(win->buffer, 250, 150, 8, 48, "CPU: x86 32-bit", 0x00FFFFFF, 0x00111155);
        draw_text_to_buffer(win->buffer, 250, 150, 8, 64, "GUI: Double Buffered", 0x00FFFFFF, 0x00111155);
    }
}

/* ------------------------------------------------------------------ */
/* desktop_start                                                      */
/* ------------------------------------------------------------------ */
void desktop_start(void) {
    if (!fb_available()) return;

    wm_init();
    compositor_init();

    /* Create a couple of demo windows for the compositor to draw */
    desktop_create_terminal();
    desktop_create_explorer();

    /* We render once immediately */
    compositor_update();
}
