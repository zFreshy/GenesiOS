/*
 * kernel/arch/x86_64/gdt.c
 * Installs a proper 64-bit GDT with null, kernel code/data,
 * user code/data, and a TSS descriptor.
 */
#include "gdt.h"
#include "../../include/kernel.h"

/* Kernel stack used by TSS (ring3 -> ring0 transitions) */
static uint8_t s_tss_stack[8192] ALIGNED(16);

static gdt_table_t s_gdt;
static gdt_ptr_t   s_gdt_ptr;
static tss_t       s_tss;

/* ------------------------------------------------------------------ */
/* Internal: fill one 8-byte entry                                     */
/* ------------------------------------------------------------------ */
static void set_entry(gdt_entry_t *e,
                      uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t gran)
{
    e->base_low  = (uint16_t)(base & 0xFFFF);
    e->base_mid  = (uint8_t)((base >> 16) & 0xFF);
    e->base_high = (uint8_t)((base >> 24) & 0xFF);
    e->limit_low = (uint16_t)(limit & 0xFFFF);
    e->gran      = (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));
    e->access    = access;
}

/* ------------------------------------------------------------------ */
/* Internal: fill 16-byte TSS descriptor                               */
/* ------------------------------------------------------------------ */
static void set_tss_entry(uint64_t base, uint32_t limit) {
    gdt_tss_entry_t *e = &s_gdt.tss;
    e->limit_low  = (uint16_t)(limit & 0xFFFF);
    e->base_low   = (uint16_t)(base & 0xFFFF);
    e->base_mid   = (uint8_t)((base >> 16) & 0xFF);
    e->access     = 0x89;   /* P=1, DPL=0, Type=TSS available (64-bit) */
    e->gran       = (uint8_t)((limit >> 16) & 0x0F);
    e->base_high  = (uint8_t)((base >> 24) & 0xFF);
    e->base_upper = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    e->reserved   = 0;
}

/* ------------------------------------------------------------------ */
/* gdt_set_tss_rsp0 — update RSP0 (called by scheduler on task switch) */
/* ------------------------------------------------------------------ */
void gdt_set_tss_rsp0(uint64_t rsp) {
    s_tss.rsp0 = rsp;
}

/* ------------------------------------------------------------------ */
/* gdt_init                                                            */
/* ------------------------------------------------------------------ */
void gdt_init(void) {
    /* Zero everything */
    kmemset(&s_gdt, 0, sizeof(s_gdt));
    kmemset(&s_tss, 0, sizeof(s_tss));

    /* Set up TSS */
    s_tss.rsp0       = (uint64_t)(s_tss_stack + sizeof(s_tss_stack));
    s_tss.iomap_base = (uint16_t)sizeof(tss_t);

    /*
     * Access byte layout:  P DPL(2) S E DC RW A
     * Gran   byte layout:  G D/B L AVL | limit[19:16]
     *
     * 64-bit kernel code: access=0x9A, gran=0xA0
     * 64-bit kernel data: access=0x92, gran=0xA0
     * 64-bit user   code: access=0xFA, gran=0xA0  (DPL=3)
     * 64-bit user   data: access=0xF2, gran=0xA0  (DPL=3)
     *
     * gran 0xA0 = G=1 D=0 L=1 AVL=0 | limit-high=0
     * (L=1 required for 64-bit code; D must be 0 when L=1)
     */
    set_entry(&s_gdt.entries[0], 0, 0,       0x00, 0x00); /* null        */
    set_entry(&s_gdt.entries[1], 0, 0xFFFFF, 0x9A, 0xA0); /* kernel code */
    set_entry(&s_gdt.entries[2], 0, 0xFFFFF, 0x92, 0x80); /* kernel data */
    set_entry(&s_gdt.entries[3], 0, 0xFFFFF, 0xF2, 0x80); /* user data   */
    set_entry(&s_gdt.entries[4], 0, 0xFFFFF, 0xFA, 0xA0); /* user code   */

    set_tss_entry((uint64_t)&s_tss, (uint32_t)sizeof(s_tss) - 1);

    /* Build GDTR */
    s_gdt_ptr.limit = (uint16_t)(sizeof(s_gdt) - 1);
    s_gdt_ptr.base  = (uint64_t)&s_gdt;

    /* Load GDTR and reload segment registers (assembly) */
    gdt_flush((uint64_t)&s_gdt_ptr);

    /* Load Task Register */
    tss_flush();
}
