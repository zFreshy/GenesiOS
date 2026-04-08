/*
 * kernel/gui/window.h
 * Basic window manager structures.
 */
#ifndef WINDOW_H
#define WINDOW_H

#include "../include/kernel.h"

#define WINDOW_MAX_TITLE 64

struct window;

typedef struct window {
    int32_t   x, y;             /* Screen position */
    uint32_t  width, height;
    char      title[WINDOW_MAX_TITLE];
    uint32_t *buffer;           /* Backing pixel buffer (ARGB) */
    
    void (*on_key)(struct window *win, char c);
    
    struct window *next;        /* Linked list for Z-order (bottom to top) */
    struct window *prev;
} window_t;

/* Initialize window manager list */
void wm_init(void);

/* Create a new window */
window_t *wm_create_window(int32_t x, int32_t y, uint32_t width, uint32_t height, const char *title);

/* Destroy a window and free its buffer */
void wm_destroy_window(window_t *win);

/* Get the bottom-most window (start of Z-order list) */
window_t *wm_get_bottom(void);

/* Get the top-most window (end of Z-order list) */
window_t *wm_get_top(void);

/* Bring a window to the top of the Z-order */
void wm_bring_to_front(window_t *win);

/* Update a region of the window's buffer (placeholder for future partial redraws) */
void wm_invalidate(window_t *win, int32_t x, int32_t y, uint32_t w, uint32_t h);

#endif /* WINDOW_H */
