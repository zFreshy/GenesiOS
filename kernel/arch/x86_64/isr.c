/*
 * kernel/arch/x86_64/isr.c
 * ISR handler registry + PIC 8259 EOI management.
 */
#include "isr.h"
#include "../../include/kernel.h"

/* Per-vector C handler table */
isr_handler_t g_isr_handlers[IDT_ENTRIES];

/* ------------------------------------------------------------------ */
/* PIC EOI — must be sent after every IRQ                              */
/* ------------------------------------------------------------------ */
#define PIC1_CMD   0x20
#define PIC2_CMD   0xA0
#define PIC_EOI    0x20

void isr_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);   /* slave  PIC */
    outb(PIC1_CMD, PIC_EOI);                  /* master PIC */
}

/* ------------------------------------------------------------------ */
/* isr_register_handler                                                */
/* ------------------------------------------------------------------ */
void isr_register_handler(uint8_t n, isr_handler_t handler) {
    g_isr_handlers[n] = handler;
}
