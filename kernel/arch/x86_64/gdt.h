/*
 * kernel/arch/x86_64/gdt.h
 * Global Descriptor Table — x86-64 flat model.
 */
#ifndef GDT_H
#define GDT_H

#include "../../include/kernel.h"

/* Segment selectors */
#define GDT_NULL         0x00
#define GDT_KERNEL_CODE  0x08
#define GDT_KERNEL_DATA  0x10
#define GDT_USER_DATA    0x18
#define GDT_USER_CODE    0x20
#define GDT_TSS          0x28   /* occupies two 8-byte slots */

/* ------------------------------------------------------------------ */
/* Descriptor structures                                               */
/* ------------------------------------------------------------------ */
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;        /* P DPL S E DC RW A */
    uint8_t  gran;          /* G D/B L AVL | limit[19:16] */
    uint8_t  base_high;
} PACKED gdt_entry_t;

/* TSS descriptor in 64-bit mode is 16 bytes (two slots) */
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} PACKED gdt_tss_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} PACKED gdt_ptr_t;

/* Task State Segment — needed for RSP0 (kernel stack on ring-3→0) */
typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;          /* Stack ptr for privilege level 0 */
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];        /* Interrupt Stack Table */
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} PACKED tss_t;

/* ------------------------------------------------------------------ */
/* GDT table layout (5 descriptors + 1 TSS = 7 slots used as 5 + 2)  */
/* ------------------------------------------------------------------ */
typedef struct {
    gdt_entry_t     entries[5];   /* null, kcode, kdata, udata, ucode */
    gdt_tss_entry_t tss;
} PACKED gdt_table_t;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
void gdt_init(void);
void gdt_set_tss_rsp0(uint64_t rsp);

/* Assembly: load GDTR and far-return to reload CS */
extern void gdt_flush(uint64_t gdt_ptr_addr);
/* Assembly: load TSS register */
extern void tss_flush(void);

#endif /* GDT_H */
