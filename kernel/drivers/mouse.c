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
#include "../include/kprintf.h"

/* ------------------------------------------------------------------ */
/* PS/2 controller I/O ports                                          */
/* ------------------------------------------------------------------ */
#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_CMD     0x64

/* ------------------------------------------------------------------ */
/* VMMouse (VMware/QEMU Absolute Mouse) Constants                     */
/* ------------------------------------------------------------------ */
#define VMMOUSE_MAGIC 0x564D5868
#define VMMOUSE_PORT  0x5658

#define VMMOUSE_CMD_GETVERSION          10
#define VMMOUSE_CMD_ABSPOINTER_DATA     39
#define VMMOUSE_CMD_ABSPOINTER_STATUS   40
#define VMMOUSE_CMD_ABSPOINTER_COMMAND  41

#define VMMOUSE_CMD_ENABLE              0x45414552
#define VMMOUSE_CMD_REQUEST_ABSOLUTE    0x53424152

#define VMMOUSE_VERSION_ID              0x3442554a

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */
static int32_t s_x       = 0;
static int32_t s_y       = 0;
static uint8_t s_buttons = 0;
static uint8_t s_packet[3];
static uint8_t s_phase   = 0;
static bool    s_vmmouse = false;

/* Mouse screen bounds (updated on init from framebuffer size) */
static int32_t s_max_x = 1023;
static int32_t s_max_y = 767;

void mouse_set_bounds(uint32_t w, uint32_t h) {
    s_max_x = w - 1;
    s_max_y = h - 1;
    
    /* Centraliza a seta caso estejamos em modo relativo (se absoluto, ele se corrige no primeiro pacote) */
    s_x = w / 2;
    s_y = h / 2;
}

/* ------------------------------------------------------------------ */
/* VMMouse helpers                                                      */
/* ------------------------------------------------------------------ */
static void vmmouse_cmd(uint32_t cmd, uint32_t arg, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    uint32_t out_eax, out_ebx, out_ecx, out_edx;
    uint32_t dummy_esi, dummy_edi;
    __asm__ volatile (
        "inl %%dx, %%eax"
        : "=a"(out_eax), "=b"(out_ebx), "=c"(out_ecx), "=d"(out_edx), "=S"(dummy_esi), "=D"(dummy_edi)
        : "a"(VMMOUSE_MAGIC), "b"(arg), "c"(cmd), "d"(VMMOUSE_PORT)
        : "memory"
    );
    if (eax) *eax = out_eax;
    if (ebx) *ebx = out_ebx;
    if (ecx) *ecx = out_ecx;
    if (edx) *edx = out_edx;
}

static bool vmmouse_init_device(void) {
    uint32_t eax, ebx, ecx, edx;
    vmmouse_cmd(VMMOUSE_CMD_GETVERSION, 0, &eax, &ebx, &ecx, &edx);
    if (eax != VMMOUSE_VERSION_ID) {
        return false;
    }

    /* Drain any pending packets in the queue before enabling */
    uint32_t status, dummy1, dummy2, dummy3;
    vmmouse_cmd(VMMOUSE_CMD_ABSPOINTER_STATUS, 0, &status, &dummy1, &dummy2, &dummy3);
    int queue_length = status & 0xFFFF;
    while (queue_length >= 4) {
        vmmouse_cmd(VMMOUSE_CMD_ABSPOINTER_DATA, 4, &status, &dummy1, &dummy2, &dummy3);
        queue_length -= 4;
    }

    /* Enable VMMouse */
    vmmouse_cmd(VMMOUSE_CMD_ABSPOINTER_COMMAND, VMMOUSE_CMD_ENABLE, &eax, &ebx, &ecx, &edx);
    
    /* Request absolute coordinates */
    vmmouse_cmd(VMMOUSE_CMD_ABSPOINTER_COMMAND, VMMOUSE_CMD_REQUEST_ABSOLUTE, &eax, &ebx, &ecx, &edx);
    
    return true;
}

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

    /* Empty PS/2 buffer to prevent out-of-sync packets (ACKs causing drift) */
    while (inb(PS2_STATUS) & 0x01) {
        inb(PS2_DATA);
    }

    s_phase = 0;

    /* Try to initialize VMMouse (Absolute Mouse mode for QEMU/VMware) */
    if (vmmouse_init_device()) {
        s_vmmouse = true;
        kprintf("VMMouse (Absolute Pointer) enabled!\n");
    } else {
        kprintf("Standard PS/2 Mouse (Relative) enabled.\n");
    }
}

/* ------------------------------------------------------------------ */
/* mouse_irq_handler — called from IRQ12                               */
/* ------------------------------------------------------------------ */
void mouse_irq_handler(void) {
    uint8_t data = inb(PS2_DATA);

    if (s_vmmouse) {
        uint32_t status, dummy1, dummy2, dummy3;
        vmmouse_cmd(VMMOUSE_CMD_ABSPOINTER_STATUS, 0, &status, &dummy1, &dummy2, &dummy3);
        
        int queue_length = status & 0xFFFF;
        while (queue_length >= 4) {
            uint32_t pkt_status, x, y, z;
            vmmouse_cmd(VMMOUSE_CMD_ABSPOINTER_DATA, 4, &pkt_status, &x, &y, &z);
            
            if (fb_available()) {
                /* Check if the hypervisor sent a relative packet instead of absolute */
                if (pkt_status & 0x00010000) {
                    /* Inverted for WSLg / SDL relative mouse bugs */
                    s_x -= (int32_t)x;
                    s_y += (int32_t)y;
                } else {
                    /* Volta para a escala oficial do VMMouse (0 a 0xFFFF)
                     * e compensa o fato da imagem da seta começar um pouco
                     * mais para a esquerda/cima que o hotspot do cursor do host. */
                    int32_t offset_x = -2;
                    int32_t offset_y = -2;
                    
                    s_x = (int32_t)((x * fb_width()) / 0xFFFF) + offset_x;
                    s_y = (int32_t)((y * fb_height()) / 0xFFFF) + offset_y;
                }
            }
            
            s_buttons = 0;
            if (pkt_status & 0x20) s_buttons |= MOUSE_BTN_LEFT;
            if (pkt_status & 0x10) s_buttons |= MOUSE_BTN_RIGHT;
            if (pkt_status & 0x08) s_buttons |= MOUSE_BTN_MIDDLE;
            
            queue_length -= 4;
        }
        
        /* Clamp to screen */
        if (s_x < 0)       s_x = 0;
        if (s_x > s_max_x) s_x = s_max_x;
        if (s_y < 0)       s_y = 0;
        if (s_y > s_max_y) s_y = s_max_y;
        
        return; /* Skip standard PS/2 processing */
    }

    s_packet[s_phase] = data;

    if (s_phase == 0) {
        /* First byte must have bit 3 set; discard if not (re-sync).
         * Also discard ACK bytes (0xFA) that sometimes sneak in and shift the state machine.
         */
        if (data == 0xFA || !(data & 0x08)) return;
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

    /* Y axis is inverted in PS/2. 
     * Inverted X and Y again to workaround WSLg/SDL relative mouse bugs */
    s_x -= dx;
    s_y += dy;

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
