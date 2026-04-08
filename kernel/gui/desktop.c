/*
 * kernel/gui/desktop.c
 * Basic OS desktop layout and window creation.
 */
#include "desktop.h"
#include "window.h"
#include "compositor.h"
#include "../gfx/framebuffer.h"
#include "../mm/heap.h"
#include "../gfx/font.h"

static window_t *s_win1 = NULL;
static window_t *s_win2 = NULL;

void desktop_create_terminal(void) {
    window_t *win = wm_create_window(100, 100, 400, 250, "Terminal");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 400 * 250; i++) {
            win->buffer[i] = 0x00181818; /* Dark grey */
        }
        font_draw_string_to_buffer(win->buffer, 400, 250, 16, 16, "genesi> run test_app", 0x00FFFFFF, 0x00181818);
        font_draw_string_to_buffer(win->buffer, 400, 250, 16, 32, "Executing...", 0x00AAAAAA, 0x00181818);
        font_draw_string_to_buffer(win->buffer, 400, 250, 16, 48, "Hello from Ring 3!", 0x0055FF55, 0x00181818);
    }
}

void desktop_create_explorer(void) {
    window_t *win = wm_create_window(250, 150, 500, 350, "File Explorer");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 500 * 350; i++) {
            win->buffer[i] = 0x00202020; /* Dark Mica */
        }
        /* Top bar for File Explorer */
        for (uint32_t y = 0; y < 40; y++) {
            for (uint32_t x = 0; x < 500; x++) {
                win->buffer[y * 500 + x] = 0x001A1A1A; /* Darker header */
            }
        }
        font_draw_string_to_buffer(win->buffer, 500, 350, 16, 16, "C:\\Genesi\\System32", 0x00E0E0E0, 0x001A1A1A);
        font_draw_string_to_buffer(win->buffer, 500, 350, 24, 60, "\x04 Kernel", 0x00E0E0E0, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 500, 350, 24, 80, "\x04 User", 0x00E0E0E0, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 500, 350, 48, 100, "\x09 hello.txt", 0x00AAAAAA, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 500, 350, 48, 120, "\x09 test_app.elf", 0x00AAAAAA, 0x00202020);
    }
}

void desktop_create_sysinfo(void) {
    window_t *win = wm_create_window(400, 200, 300, 200, "System Info");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 300 * 200; i++) {
            win->buffer[i] = 0x00202020; /* Dark Mica */
        }
        /* Accent header */
        for (uint32_t y = 0; y < 8; y++) {
            for (uint32_t x = 0; x < 300; x++) {
                win->buffer[y * 300 + x] = 0x000078D7; /* Windows Blue */
            }
        }
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 24, "Genesi OS v11", 0x00FFFFFF, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 48, "Memory: 256 MB", 0x00AAAAAA, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 64, "CPU: x86 64-bit", 0x00AAAAAA, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 80, "GUI: W11 Dark Theme", 0x00AAAAAA, 0x00202020);
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
