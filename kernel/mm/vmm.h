/*
 * kernel/mm/vmm.h
 * Virtual Memory Manager — 4-level paging (PML4) for x86-64.
 */
#ifndef VMM_H
#define VMM_H

#include "../include/kernel.h"

/* Page flags (low 12 bits of a page table entry) */
#define VMM_PRESENT   (1ULL << 0)
#define VMM_WRITABLE  (1ULL << 1)
#define VMM_USER      (1ULL << 2)
#define VMM_HUGE      (1ULL << 7)   /* 2 MB or 1 GB page */
#define VMM_NX        (1ULL << 63)  /* No-execute (requires EFER.NXE) */

typedef uint64_t pte_t;

void vmm_init(void);

/* Map a virtual address to a physical address */
void vmm_map(uint64_t virt, uint64_t phys, uint64_t flags);

/* Map a virtual address to a physical address for user-space */
void vmm_map_user(uint64_t pml4_phys, uint64_t virt, uint64_t phys, uint64_t flags);

/* Unmap a virtual address */
void vmm_unmap(uint64_t virt);

/* Get physical address of a virtual address (0 if not mapped) */
uint64_t vmm_get_phys(uint64_t virt);

/* Create a new address space (PML4) by copying kernel mappings */
uint64_t vmm_create_address_space(void);

/* Switch to a page table (load CR3) */
void vmm_load_cr3(uint64_t pml4_phys);

/* Get current CR3 */
uint64_t vmm_get_cr3(void);

#endif /* VMM_H */
