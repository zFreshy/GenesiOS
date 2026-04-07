/*
 * kernel/arch/x86_64/idt.h
 * Interrupt Descriptor Table — 64-bit interrupt gates.
 */
#ifndef IDT_H
#define IDT_H

#include "../../include/kernel.h"

#define IDT_ENTRIES 256

/* ------------------------------------------------------------------ */
/* 64-bit interrupt gate descriptor                                    */
/* ------------------------------------------------------------------ */
typedef struct {
    uint16_t offset_low;    /* handler bits  0-15  */
    uint16_t selector;      /* code segment selector (GDT_KERNEL_CODE) */
    uint8_t  ist;           /* Interrupt Stack Table index (0 = no switch) */
    uint8_t  type_attr;     /* P DPL 0 Type(4) */
    uint16_t offset_mid;    /* handler bits 16-31  */
    uint32_t offset_high;   /* handler bits 32-63  */
    uint32_t reserved;      /* must be 0 */
} PACKED idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} PACKED idt_ptr_t;

/*
 * type_attr for a 64-bit interrupt gate at ring 0:
 *   P=1, DPL=00, 0, Type=1110 => 0x8E
 * For a trap gate (does NOT clear IF):
 *   Type=1111 => 0x8F
 */
#define IDT_INTERRUPT_GATE  0x8E
#define IDT_TRAP_GATE       0x8F

/* ------------------------------------------------------------------ */
/* CPU exception / IRQ register frame pushed by isr_common            */
/* ------------------------------------------------------------------ */
typedef struct {
    /* Saved general-purpose registers (pushed by isr_common, high→low) */
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    /* Pushed by ISR stub */
    uint64_t int_no;
    uint64_t err_code;
    /* Pushed automatically by the CPU on interrupt */
    uint64_t rip, cs, rflags, rsp, ss;
} PACKED registers_t;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
void idt_init(void);
void idt_set_gate(uint8_t n, uint64_t handler, uint8_t type_attr, uint8_t ist);

/* C-level interrupt dispatcher — called from isr_common (ASM) */
void isr_handler(registers_t *regs);

/* Assembly: load IDTR */
extern void idt_flush(uint64_t idt_ptr_addr);

#endif /* IDT_H */
