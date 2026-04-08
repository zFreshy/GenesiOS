/*
 * kernel/drivers/mouse.c
 * PS/2 mouse driver — IRQ12 handler, 3-byte packet state machine.
 *
 * PS/2 protocol:
 *   Byte 0: YO XO YS XS 1 MC MR ML  (overflow+sign+button flags)
 *   Byte 1: X movement (signed, with XS as sign)
 *   Byte 2: Y movement (signed, with YS as sign, Y-axis inverted)
 */
#include "mouse.h"
#include "../gfx/framebuffer.h"
#include "../include/kernel.h"

/* ------------------------------------------------------------------ */
/* PS/2 controller I/O ports                                          */
/* ------------------------------------------------------------------ */
#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_CMD     0x64

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */
static int32_t s_x       = 0;
static int32_t s_y       = 0;
static uint8_t s_buttons = 0;
static uint8_t s_packet[3];
static uint8_t s_phase   = 0;

/* Mouse screen bounds (updated on init from framebuffer size) */
static int32_t s_max_x = 1023;
static int32_t s_max_y = 767;

/* ------------------------------------------------------------------ */
/* PS/2 helpers                                                         */
/* ------------------------------------------------------------------ */
static void ps2_wait_write(void) {
    int timeout = 100000;
    while (timeout-- && (inb(PS2_STATUS) & 0x02));
}
static void ps2_wait_read(void) {
    int timeout = 100000;
    while (timeout-- && !(inb(PS2_STATUS) & 0x01));
}
static void mouse_write(uint8_t data) {
    ps2_wait_write();
    outb(PS2_CMD, 0xD4);   /* next byte goes to mouse */
    ps2_wait_write();
    outb(PS2_DATA, data);
}
static uint8_t mouse_read(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

/* ------------------------------------------------------------------ */
/* mouse_init                                                           */
/* ------------------------------------------------------------------ */
void mouse_init(void) {
    /* Update bounds from framebuffer */
    if (fb_available()) {
        s_x     = (int32_t)fb_width()  / 2;
        s_y     = (int32_t)fb_height() / 2;
        s_max_x = (int32_t)fb_width()  - 1;
        s_max_y = (int32_t)fb_height() - 1;
    }

    /* Enable PS/2 auxiliary (mouse) port */
    ps2_wait_write();
    outb(PS2_CMD, 0xA8);

    /* Enable mouse interrupt in the PS/2 controller */
    ps2_wait_write();
    outb(PS2_CMD, 0x20);       /* read current config  */
    ps2_wait_read();
    uint8_t cfg = inb(PS2_DATA);
    cfg |= 0x02;               /* enable IRQ12         */
    cfg &= ~0x20;              /* clear "mouse disable" bit */
    ps2_wait_write();
    outb(PS2_CMD, 0x60);
    ps2_wait_write();
    outb(PS2_DATA, cfg);

    /* Tell mouse to use default settings */
    mouse_write(0xF6);
    mouse_read();   /* ACK */

    /* Enable mouse packet streaming */
    mouse_write(0xF4);
    mouse_read();   /* ACK */

    s_phase = 0;
}

/* ------------------------------------------------------------------ */
/* mouse_irq_handler — called from IRQ12                               */
/* ------------------------------------------------------------------ */
void mouse_irq_handler(void) {
    uint8_t data = inb(PS2_DATA);

    s_packet[s_phase] = data;

    if (s_phase == 0) {
        /* First byte must have bit 3 set; discard if not (re-sync) */
        if (!(data & 0x08)) return;
    }

    s_phase++;
    if (s_phase < 3) return;

    /* Full packet received */
    s_phase = 0;

    uint8_t  flags = s_packet[0];
    int32_t  dx    = (int32_t)(int8_t)s_packet[1];
    int32_t  dy    = (int32_t)(int8_t)s_packet[2];

    /* Discard overflowed packets */
    if (flags & 0xC0) return;

    s_buttons = flags & 0x07;

    /* Y axis is inverted in PS/2 */
    s_x += dx;
    s_y -= dy;

    /* Clamp to screen */
    if (s_x < 0)       s_x = 0;
    if (s_x > s_max_x) s_x = s_max_x;
    if (s_y < 0)       s_y = 0;
    if (s_y > s_max_y) s_y = s_max_y;
}

/* ------------------------------------------------------------------ */
/* Accessors                                                            */
/* ------------------------------------------------------------------ */
int32_t mouse_x(void)       { return s_x; }
int32_t mouse_y(void)       { return s_y; }
uint8_t mouse_buttons(void) { return s_buttons; }

/* ------------------------------------------------------------------ */
/* mouse_draw_cursor — draw a simple 11×11 cross cursor               */
/* ------------------------------------------------------------------ */
void mouse_draw_cursor(uint32_t colour) {
    if (!fb_available()) return;
    int32_t cx = s_x, cy = s_y;
    for (int i = -5; i <= 5; i++) {
        fb_putpixel((uint32_t)(cx + i), (uint32_t)cy,      colour);
        fb_putpixel((uint32_t)cx,       (uint32_t)(cy + i), colour);
    }
}
