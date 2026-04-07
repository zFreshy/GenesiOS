/*
 * kernel/mm/pmm.c
 * Physical Memory Manager.
 * Uses a static bitmap: 1 bit per 4 KB frame.
 * Supports up to 4 GB (128 K frames, 16 KB of bitmap).
 */
#include "pmm.h"
#include "../include/kprintf.h"

#define MAX_FRAMES  (4ULL * 1024 * 1024 * 1024 / PAGE_SIZE)   /* 1 M frames */
#define BITMAP_QWORDS (MAX_FRAMES / 64)

/*
 * Bitmap: bit = 0 -> free, bit = 1 -> used.
 * We mark everything as used at first, then free usable regions.
 */
static uint64_t s_bitmap[BITMAP_QWORDS];
static uint64_t s_total  = 0;
static uint64_t s_free   = 0;

/* kernel symbol from linker.ld */
extern uint8_t kernel_end;

/* ------------------------------------------------------------------ */
/* Bitmap helpers                                                       */
/* ------------------------------------------------------------------ */
static inline void bitmap_set(uint64_t frame) {
    s_bitmap[frame / 64] |= (1ULL << (frame % 64));
}
static inline void bitmap_clear(uint64_t frame) {
    s_bitmap[frame / 64] &= ~(1ULL << (frame % 64));
}
static inline bool bitmap_test(uint64_t frame) {
    return (s_bitmap[frame / 64] >> (frame % 64)) & 1;
}

/* ------------------------------------------------------------------ */
/* pmm_init — parse Multiboot2 memory map and build the bitmap         */
/* ------------------------------------------------------------------ */
void pmm_init(uint64_t mboot_info) {
    /* Start with everything marked used */
    for (size_t i = 0; i < BITMAP_QWORDS; i++) s_bitmap[i] = ~0ULL;

    mb2_tag_t *tag = mb2_find_tag(mboot_info, MB2_TAG_MMAP);
    if (!tag) {
        kpanic("PMM: no memory map from bootloader!\n");
    }

    mb2_mmap_tag_t   *mmap = (mb2_mmap_tag_t *)tag;
    mb2_mmap_entry_t *e    = (mb2_mmap_entry_t *)((uint8_t *)mmap + sizeof(mb2_mmap_tag_t));
    mb2_mmap_entry_t *end  = (mb2_mmap_entry_t *)((uint8_t *)mmap + mmap->size);

    uint64_t kernel_end_addr = ALIGN_UP((uint64_t)(uintptr_t)&kernel_end, PAGE_SIZE);

    while (e < end) {
        uint64_t base   = e->base_addr;
        uint64_t length = e->length;

        s_total += length / PAGE_SIZE;

        if (e->type == MB2_MMAP_AVAILABLE && length >= PAGE_SIZE) {
            uint64_t frame_start = ALIGN_UP(base, PAGE_SIZE) >> PAGE_SHIFT;
            uint64_t frame_end   = (base + length) >> PAGE_SHIFT;

            for (uint64_t f = frame_start; f < frame_end && f < MAX_FRAMES; f++) {
                /* Keep frames used by kernel, bitmap and first 1 MB reserved */
                uint64_t addr = FRAME_TO_ADDR(f);
                if (addr < 0x100000) goto next_frame;   /* first 1 MB: reserved */
                if (addr >= (uint64_t)(uintptr_t)s_bitmap &&
                    addr <  (uint64_t)(uintptr_t)(s_bitmap + BITMAP_QWORDS)) goto next_frame;
                if (addr >= 0x100000 && addr < kernel_end_addr) goto next_frame;

                bitmap_clear(f);
                s_free++;
                next_frame:;
            }
        }
        e = (mb2_mmap_entry_t *)((uint8_t *)e + mmap->entry_size);
    }

    kprintf("[PMM] Total: %zu MB | Free: %zu MB\n",
            (size_t)(s_total * PAGE_SIZE / (1024*1024)),
            (size_t)(s_free  * PAGE_SIZE / (1024*1024)));
}

/* ------------------------------------------------------------------ */
/* pmm_alloc_frame — first-fit search                                   */
/* ------------------------------------------------------------------ */
uint64_t pmm_alloc_frame(void) {
    for (uint64_t i = 0; i < BITMAP_QWORDS; i++) {
        if (s_bitmap[i] == ~0ULL) continue;    /* all used — skip */
        /* Find first free bit */
        uint64_t val = ~s_bitmap[i];           /* 1 = free */
        uint64_t bit = (uint64_t)__builtin_ctzll(val);
        uint64_t frame = i * 64 + bit;
        bitmap_set(frame);
        s_free--;
        return FRAME_TO_ADDR(frame);
    }
    return 0;   /* Out of memory */
}

/* ------------------------------------------------------------------ */
/* pmm_free_frame                                                       */
/* ------------------------------------------------------------------ */
void pmm_free_frame(uint64_t addr) {
    uint64_t frame = ADDR_TO_FRAME(addr);
    if (bitmap_test(frame)) {
        bitmap_clear(frame);
        s_free++;
    }
}

uint64_t pmm_free_frames(void)  { return s_free; }
uint64_t pmm_total_frames(void) { return s_total; }
