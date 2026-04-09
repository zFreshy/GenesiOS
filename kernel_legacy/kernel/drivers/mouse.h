/*
 * kernel/drivers/mouse.h
 * PS/2 mouse driver — IRQ12, 3-byte packets, cursor tracking.
 */
#ifndef MOUSE_H
#define MOUSE_H

#include "../include/kernel.h"

/* Current mouse state */
int32_t mouse_x(void);
int32_t mouse_y(void);
uint8_t mouse_buttons(void); /* bit0=left, bit1=right, bit2=middle */

#define MOUSE_BTN_LEFT   (1 << 0)
#define MOUSE_BTN_RIGHT  (1 << 1)
#define MOUSE_BTN_MIDDLE (1 << 2)

/* Initialise the PS/2 mouse (enables aux port, sets IRQ12 mask) */
void mouse_init(void);

/* Called by IRQ12 handler — feeds a data byte into the packet state machine */
void mouse_irq_handler(void);

/* Draw a small cursor cross on the framebuffer */
void mouse_draw_cursor(uint32_t colour);

#endif /* MOUSE_H */
