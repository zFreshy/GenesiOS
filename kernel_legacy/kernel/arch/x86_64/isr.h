/*
 * kernel/arch/x86_64/isr.h
 * ISR / IRQ handler registration interface.
 */
#ifndef ISR_H
#define ISR_H

#include "idt.h"

/* Handler function pointer type */
typedef void (*isr_handler_t)(registers_t *regs);

/* ------------------------------------------------------------------ */
/* Register a C handler for a given interrupt vector                   */
/* ------------------------------------------------------------------ */
void isr_register_handler(uint8_t n, isr_handler_t handler);

/* Exception names (for debug output) */
extern const char *g_exception_names[32];

/* ------------------------------------------------------------------ */
/* Extern declarations for all ISR stubs (defined in isr.asm)          */
/* ------------------------------------------------------------------ */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

#endif /* ISR_H */
