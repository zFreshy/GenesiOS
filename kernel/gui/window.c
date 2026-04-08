/*
 * kernel/gui/window.c
 * Basic window manager implementation.
 */
#include "window.h"
#include "../mm/heap.h"

static window_t *s_bottom_window = NULL;
static window_t *s_top_window    = NULL;

/* ------------------------------------------------------------------ */
/* Initialize window manager                                          */
/* ------------------------------------------------------------------ */
void wm_init(void) {
    s_bottom_window = NULL;
    s_top_window    = NULL;
}

/* ------------------------------------------------------------------ */
/* Create a new window                                                */
/* ------------------------------------------------------------------ */
window_t *wm_create_window(int32_t x, int32_t y, uint32_t width, uint32_t height, const char *title) {
    window_t *win = (window_t *)kmalloc(sizeof(window_t));
    if (!win) return NULL;

    win->x      = x;
    win->y      = y;
    win->width  = width;
    win->height = height;
    
    /* Copy title */
    int i = 0;
    while (title && title[i] && i < WINDOW_MAX_TITLE - 1) {
        win->title[i] = title[i];
        i++;
    }
    win->title[i] = '\0';

    /* Allocate pixel buffer */
    win->buffer = (uint32_t *)kmalloc(width * height * sizeof(uint32_t));
    if (!win->buffer) {
        kfree(win);
        return NULL;
    }
    kmemset(win->buffer, 0, width * height * sizeof(uint32_t));

    win->on_key = NULL;

    /* Add to top of Z-order */
    win->next = NULL;
    win->prev = s_top_window;
    
    if (s_top_window) {
        s_top_window->next = win;
    } else {
        s_bottom_window = win;
    }
    s_top_window = win;

    return win;
}

/* ------------------------------------------------------------------ */
/* Destroy a window                                                   */
/* ------------------------------------------------------------------ */
void wm_destroy_window(window_t *win) {
    if (!win) return;

    if (win->prev) win->prev->next = win->next;
    else s_bottom_window = win->next;

    if (win->next) win->next->prev = win->prev;
    else s_top_window = win->prev;

    if (win->buffer) kfree(win->buffer);
    kfree(win);
}

/* ------------------------------------------------------------------ */
/* Bring a window to the top of the Z-order                           */
/* ------------------------------------------------------------------ */
void wm_bring_to_front(window_t *win) {
    if (!win || win == s_top_window) return;

    /* Remove from current position */
    if (win->prev) win->prev->next = win->next;
    else s_bottom_window = win->next;

    if (win->next) win->next->prev = win->prev;
    else s_top_window = win->prev;

    /* Append to top */
    win->next = NULL;
    win->prev = s_top_window;
    
    if (s_top_window) s_top_window->next = win;
    else s_bottom_window = win;
    
    s_top_window = win;
}

window_t *wm_get_top(void) {
    return s_top_window;
}

/* ------------------------------------------------------------------ */
/* Get bottom window (for compositor)                                 */
/* ------------------------------------------------------------------ */
window_t *wm_get_bottom(void) {
    return s_bottom_window;
}

/* ------------------------------------------------------------------ */
/* Invalidate window region (stub for future partial redraws)         */
/* ------------------------------------------------------------------ */
void wm_invalidate(window_t *win, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    (void)win; (void)x; (void)y; (void)w; (void)h;
    /* Typically notifies the compositor that a redraw is needed */
}
