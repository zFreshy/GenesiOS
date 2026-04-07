/*
 * kernel/mm/vmm.c
 * Virtual Memory Manager.
 * Extends the boot identity map and provides a proper kernel mapping.
 *
 * We continue using the identity map for now (phys == virt for kernel).
 * Future phases will add per-process address spaces.
 */
#include "vmm.h"
#include "pmm.h"
#include "../include/kprintf.h"

/* Indices into each paging level from a virtual address */
#define PML4_IDX(v) (((v) >> 39) & 0x1FF)
#define PDP_IDX(v)  (((v) >> 30) & 0x1FF)
#define PD_IDX(v)   (((v) >> 21) & 0x1FF)
#define PT_IDX(v)   (((v) >> 12) & 0x1FF)
#define PAGE_ADDR(e) ((e) & ~0xFFFULL & ~(1ULL<<63))

/* ------------------------------------------------------------------ */
/* Get or allocate a child table                                        */
/* ------------------------------------------------------------------ */
static pte_t *get_or_create_table(pte_t *parent, size_t idx, uint64_t flags) {
    if (parent[idx] & VMM_PRESENT) {
        return (pte_t *)(uintptr_t)PAGE_ADDR(parent[idx]);
    }
    uint64_t phys = pmm_alloc_frame();
    if (!phys) kpanic("vmm: out of physical memory for page table\n");
    kmemset((void *)(uintptr_t)phys, 0, PAGE_SIZE);
    parent[idx] = phys | flags | VMM_PRESENT;
    return (pte_t *)(uintptr_t)phys;
}

/* ------------------------------------------------------------------ */
/* vmm_init — extend the identity map to cover all available RAM       */
/* ------------------------------------------------------------------ */
void vmm_init(void) {
    /* The boot identity map (first 16 MB) is already in place from boot.asm.
     * For now just report that VMM is active. */
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    kprintf("[VMM] CR3=0x%p  identity map active\n", (void *)cr3);
}

/* ------------------------------------------------------------------ */
/* vmm_map                                                              */
/* ------------------------------------------------------------------ */
void vmm_map(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    pte_t *pml4 = (pte_t *)(uintptr_t)(cr3 & ~0xFFFULL);

    pte_t *pdp = get_or_create_table(pml4, PML4_IDX(virt), VMM_PRESENT | VMM_WRITABLE);
    pte_t *pd  = get_or_create_table(pdp,  PDP_IDX(virt),  VMM_PRESENT | VMM_WRITABLE);
    pte_t *pt  = get_or_create_table(pd,   PD_IDX(virt),   VMM_PRESENT | VMM_WRITABLE);

    pt[PT_IDX(virt)] = (phys & ~0xFFFULL) | (flags & 0xFFF) | VMM_PRESENT;

    /* Invalidate the TLB entry for this virtual address */
    __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

/* ------------------------------------------------------------------ */
/* vmm_unmap                                                            */
/* ------------------------------------------------------------------ */
void vmm_unmap(uint64_t virt) {
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    pte_t *pml4 = (pte_t *)(uintptr_t)(cr3 & ~0xFFFULL);

    if (!(pml4[PML4_IDX(virt)] & VMM_PRESENT)) return;
    pte_t *pdp = (pte_t *)(uintptr_t)PAGE_ADDR(pml4[PML4_IDX(virt)]);
    if (!(pdp[PDP_IDX(virt)] & VMM_PRESENT)) return;
    pte_t *pd  = (pte_t *)(uintptr_t)PAGE_ADDR(pdp[PDP_IDX(virt)]);
    if (!(pd[PD_IDX(virt)] & VMM_PRESENT)) return;
    pte_t *pt  = (pte_t *)(uintptr_t)PAGE_ADDR(pd[PD_IDX(virt)]);

    pt[PT_IDX(virt)] = 0;
    __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

/* ------------------------------------------------------------------ */
/* vmm_get_phys                                                         */
/* ------------------------------------------------------------------ */
uint64_t vmm_get_phys(uint64_t virt) {
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    pte_t *pml4 = (pte_t *)(uintptr_t)(cr3 & ~0xFFFULL);
    if (!(pml4[PML4_IDX(virt)] & VMM_PRESENT)) return 0;
    pte_t *pdp = (pte_t *)(uintptr_t)PAGE_ADDR(pml4[PML4_IDX(virt)]);
    if (!(pdp[PDP_IDX(virt)] & VMM_PRESENT)) return 0;
    pte_t *pd  = (pte_t *)(uintptr_t)PAGE_ADDR(pdp[PDP_IDX(virt)]);
    if (!(pd[PD_IDX(virt)] & VMM_PRESENT)) return 0;
    pte_t *pt  = (pte_t *)(uintptr_t)PAGE_ADDR(pd[PD_IDX(virt)]);
    if (!(pt[PT_IDX(virt)] & VMM_PRESENT)) return 0;
    return PAGE_ADDR(pt[PT_IDX(virt)]) | (virt & 0xFFF);
}

void     vmm_load_cr3(uint64_t pml4_phys) {
    __asm__ volatile ("mov %0, %%cr3" :: "r"(pml4_phys) : "memory");
}
uint64_t vmm_get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}
