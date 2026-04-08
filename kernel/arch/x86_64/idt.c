/*
 * kernel/arch/x86_64/idt.c
 * IDT setup + central interrupt dispatcher.
 */
#include "idt.h"
#include "isr.h"
#include "gdt.h"
#include "../../include/kprintf.h"
#include "../../include/kernel.h"

/* 256 IDT entries */
static idt_entry_t s_idt[IDT_ENTRIES];
static idt_ptr_t   s_idt_ptr;

/* ------------------------------------------------------------------ */
/* idt_flush — assembly stub                                           */
/* ------------------------------------------------------------------ */
void idt_flush(uint64_t addr) {
    __asm__ volatile ("lidt (%0)" :: "r"(addr) : "memory");
}

/* ------------------------------------------------------------------ */
/* idt_set_gate                                                         */
/* ------------------------------------------------------------------ */
void idt_set_gate(uint8_t n, uint64_t handler, uint8_t type_attr, uint8_t ist) {
    s_idt[n].offset_low  = (uint16_t)(handler & 0xFFFF);
    s_idt[n].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    s_idt[n].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    s_idt[n].selector    = GDT_KERNEL_CODE;
    s_idt[n].ist         = ist & 0x07;
    s_idt[n].type_attr   = type_attr;
    s_idt[n].reserved    = 0;
}

/* ------------------------------------------------------------------ */
/* Exception names                                                     */
/* ------------------------------------------------------------------ */
const char *g_exception_names[32] = {
    "Division By Zero",          "Debug",
    "Non-Maskable Interrupt",    "Breakpoint",
    "Overflow",                  "Bound Range Exceeded",
    "Invalid Opcode",            "Device Not Available",
    "Double Fault",              "Coprocessor Segment Overrun",
    "Invalid TSS",               "Segment Not Present",
    "Stack-Segment Fault",       "General Protection Fault",
    "Page Fault",                "Reserved",
    "x87 FPU Error",             "Alignment Check",
    "Machine Check",             "SIMD FP Exception",
    "Virtualization Exception",  "Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "VMM Communication",
    "Security Exception",        "Reserved",
};

/* ------------------------------------------------------------------ */
/* isr_handler — central C dispatcher (called from isr_common in ASM)  */
/* ------------------------------------------------------------------ */
void isr_handler(registers_t *regs) {
    extern isr_handler_t g_isr_handlers[IDT_ENTRIES];

    if (g_isr_handlers[regs->int_no]) {
        g_isr_handlers[regs->int_no](regs);
        return;
    }

    /* Unhandled CPU exception */
    if (regs->int_no < 32) {
        kpanic("Unhandled exception #%zu: %s\n"
               "  RIP=%p  RSP=%p  RFLAGS=%p\n"
               "  ERR=%p  RAX=%p  RBX=%p\n"
               "  CS=%p  SS=%p\n",
               (size_t)regs->int_no,
               g_exception_names[regs->int_no],
               (void *)regs->rip, (void *)regs->rsp, (void *)regs->rflags,
               (void *)regs->err_code, (void *)regs->rax, (void *)regs->rbx,
               (void *)regs->cs, (void *)regs->ss);
    }
    /* Unhandled IRQ — just ignore */
}

/* ------------------------------------------------------------------ */
/* idt_init                                                             */
/* ------------------------------------------------------------------ */
void idt_init(void) {
    kmemset(s_idt, 0, sizeof(s_idt));

    /* Install exception stubs */
    idt_set_gate(0,  (uint64_t)isr0,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(1,  (uint64_t)isr1,  IDT_TRAP_GATE,      0);
    idt_set_gate(2,  (uint64_t)isr2,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(3,  (uint64_t)isr3,  IDT_TRAP_GATE,      0);
    idt_set_gate(4,  (uint64_t)isr4,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(5,  (uint64_t)isr5,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(6,  (uint64_t)isr6,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(7,  (uint64_t)isr7,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(8,  (uint64_t)isr8,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(9,  (uint64_t)isr9,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(10, (uint64_t)isr10, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(11, (uint64_t)isr11, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(12, (uint64_t)isr12, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(13, (uint64_t)isr13, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(14, (uint64_t)isr14, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(15, (uint64_t)isr15, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(16, (uint64_t)isr16, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(17, (uint64_t)isr17, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(18, (uint64_t)isr18, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(19, (uint64_t)isr19, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(20, (uint64_t)isr20, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(21, (uint64_t)isr21, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(22, (uint64_t)isr22, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(23, (uint64_t)isr23, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(24, (uint64_t)isr24, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(25, (uint64_t)isr25, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(26, (uint64_t)isr26, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(27, (uint64_t)isr27, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(28, (uint64_t)isr28, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(29, (uint64_t)isr29, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(30, (uint64_t)isr30, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(31, (uint64_t)isr31, IDT_INTERRUPT_GATE, 0);

    /* Install IRQ stubs (mapped to vectors 32-47 after PIC remap) */
    idt_set_gate(32, (uint64_t)irq0,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(33, (uint64_t)irq1,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(34, (uint64_t)irq2,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(35, (uint64_t)irq3,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(36, (uint64_t)irq4,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(37, (uint64_t)irq5,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(38, (uint64_t)irq6,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(39, (uint64_t)irq7,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(40, (uint64_t)irq8,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(41, (uint64_t)irq9,  IDT_INTERRUPT_GATE, 0);
    idt_set_gate(42, (uint64_t)irq10, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(43, (uint64_t)irq11, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(44, (uint64_t)irq12, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(45, (uint64_t)irq13, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(46, (uint64_t)irq14, IDT_INTERRUPT_GATE, 0);
    idt_set_gate(47, (uint64_t)irq15, IDT_INTERRUPT_GATE, 0);

    s_idt_ptr.limit = (uint16_t)(sizeof(s_idt) - 1);
    s_idt_ptr.base  = (uint64_t)s_idt;

    idt_flush((uint64_t)&s_idt_ptr);
}
