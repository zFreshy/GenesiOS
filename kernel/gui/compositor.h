/*
 * kernel/gui/compositor.h
 * Desktop compositor for window manager.
 */
#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "../include/kernel.h"

/* Initialize the compositor (allocates backbuffer) */
void compositor_init(void);

/* Composite all windows and mouse cursor, then blit to screen */
void compositor_render(void);

#endif /* COMPOSITOR_H */
