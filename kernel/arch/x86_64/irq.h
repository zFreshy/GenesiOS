/*
 * kernel/arch/x86_64/irq.h
 * PIC 8259 initialization and IRQ management.
 */
#ifndef IRQ_H
#define IRQ_H

#include "../../include/kernel.h"
#include "isr.h"

/* IRQ numbers (before mapping) */
#define IRQ_TIMER     0
#define IRQ_KEYBOARD  1
#define IRQ_CASCADE   2
#define IRQ_COM2      3
#define IRQ_COM1      4
#define IRQ_RTC       8
#define IRQ_MOUSE     12
#define IRQ_ATA1      14
#define IRQ_ATA2      15

/* IRQ to interrupt vector: vector = IRQ + 32 */
#define IRQ_TO_VEC(n) ((n) + 32)

void irq_init(void);
void irq_register_handler(uint8_t irq, isr_handler_t handler);
void irq_mask(uint8_t irq);
void irq_unmask(uint8_t irq);
void irq_send_eoi(uint8_t irq);

#endif /* IRQ_H */
