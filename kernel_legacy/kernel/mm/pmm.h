/*
 * kernel/mm/pmm.h
 * Physical Memory Manager — bitmap allocator (4 KB pages).
 */
#ifndef PMM_H
#define PMM_H

#include "../include/kernel.h"
#include "../include/multiboot2.h"

#define PAGE_SIZE  4096ULL
#define PAGE_SHIFT 12

/* Convert between addresses and frame indices */
#define ADDR_TO_FRAME(a)  ((a) >> PAGE_SHIFT)
#define FRAME_TO_ADDR(f)  ((f) << PAGE_SHIFT)

void     pmm_init(uint64_t mboot_info);
uint64_t pmm_alloc_frame(void);          /* returns physical address, 0 on OOM */
uint64_t pmm_alloc_frames(size_t count); /* allocate contiguous frames */
void     pmm_free_frame(uint64_t addr);
uint64_t pmm_free_frames(void);
uint64_t pmm_total_frames(void);

#endif /* PMM_H */
