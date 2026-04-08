/*
 * kernel/arch/x86_64/irq.c
 * Remap PIC 8259 and provide IRQ management.
 *
 * After remapping:
 *   IRQ  0-7  -> vectors 32-39  (PIC1 master)
 *   IRQ  8-15 -> vectors 40-47  (PIC2 slave)
 */
#include "irq.h"

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1
#define PIC_EOI    0x20

/* ICW = Initialization Command Word */
#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

/* ------------------------------------------------------------------ */
/* irq_init — remap PIC so IRQs don't clash with CPU exceptions        */
/* ------------------------------------------------------------------ */
void irq_init(void) {
    /* Save current masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* Start initialization sequence (cascade mode) */
    outb(PIC1_CMD,  ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_CMD,  ICW1_INIT | ICW1_ICW4); io_wait();

    /* ICW2: vector offsets */
    outb(PIC1_DATA, 32); io_wait();   /* master: IRQ0 -> vector 32 */
    outb(PIC2_DATA, 40); io_wait();   /* slave:  IRQ8 -> vector 40 */

    /* ICW3: cascade identity */
    outb(PIC1_DATA, 0x04); io_wait(); /* master: slave on IRQ2 (bit 2) */
    outb(PIC2_DATA, 0x02); io_wait(); /* slave: cascade identity = 2 */

    /* ICW4: 8086 mode */
    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    /* Restore saved masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/* ------------------------------------------------------------------ */
/* irq_register_handler — convenience wrapper around isr_register      */
/* ------------------------------------------------------------------ */
void irq_register_handler(uint8_t irq, isr_handler_t handler) {
    isr_register_handler((uint8_t)(irq + 32), handler);
}

/* ------------------------------------------------------------------ */
/* IRQ masking (bit = 1 means masked/disabled)                         */
/* ------------------------------------------------------------------ */
void irq_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = (irq < 8) ? irq : (uint8_t)(irq - 8);
    outb(port, (uint8_t)(inb(port) | (uint8_t)(1 << bit)));
}

void irq_unmask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = (irq < 8) ? irq : (uint8_t)(irq - 8);
    outb(port, (uint8_t)(inb(port) & (uint8_t)~(1 << bit)));
}

/* ------------------------------------------------------------------ */
/* irq_send_eoi — send End-of-Interrupt signal                         */
/* ------------------------------------------------------------------ */
void irq_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}
