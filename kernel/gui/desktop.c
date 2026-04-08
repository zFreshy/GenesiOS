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

/* ------------------------------------------------------------------ */
/* desktop_start                                                      */
/* ------------------------------------------------------------------ */
void desktop_start(void) {
    if (!fb_available()) return;

    wm_init();
    compositor_init();

    /* Create a couple of demo windows for the compositor to draw */
    s_win1 = wm_create_window(100, 100, 300, 200, "Window 1");
    if (s_win1 && s_win1->buffer) {
        /* Paint the window's own buffer some color */
        for (uint32_t i = 0; i < 300 * 200; i++) {
            s_win1->buffer[i] = 0x00DDAAAA; /* Light red/pink */
        }
    }

    s_win2 = wm_create_window(250, 150, 400, 300, "Window 2");
    if (s_win2 && s_win2->buffer) {
        for (uint32_t i = 0; i < 400 * 300; i++) {
            s_win2->buffer[i] = 0x00AADDAA; /* Light green */
        }
    }

    /* We render once immediately, but really this should be hooked 
       into a loop or the mouse IRQ for continuous refreshing. */
    compositor_render();
}
