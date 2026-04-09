/*
 * kernel/mm/heap.c
 * Simple free-list kernel heap allocator.
 * Uses a static 4 MB BSS buffer — safe since identity map covers all of it.
 */
#include "heap.h"
#include "pmm.h"
#include "../include/kprintf.h"

/* Aumentado de 32MB para 128MB para suportar monitores 4K (3840x2160x32 = ~33MB) */
#define HEAP_INITIAL_SIZE (128U * 1024U * 1024U)   /* 128 MB */
#define HEAP_MAGIC_FREE   0xDEADC0DEULL
#define HEAP_MAGIC_USED   0xC0FFEEULL

typedef struct heap_block {
    uint64_t          magic;
    size_t            size;   /* usable bytes, excluding header */
    bool              free;
    struct heap_block *next;
} heap_block_t;

static heap_block_t *s_head = NULL;

/* ------------------------------------------------------------------ */
/* heap_init                                                            */
/* ------------------------------------------------------------------ */
void heap_init(void) {
    uint64_t pages = HEAP_INITIAL_SIZE / PAGE_SIZE;
    uint64_t phys = pmm_alloc_frames(pages);
    if (!phys) kpanic("Heap: failed to allocate initial memory\n");

    /* Identity mapped, so virtual = physical */
    s_head = (heap_block_t *)(uintptr_t)phys;
    s_head->magic = HEAP_MAGIC_FREE;
    s_head->size  = HEAP_INITIAL_SIZE - sizeof(heap_block_t);
    s_head->free  = true;
    s_head->next  = NULL;
    kprintf("[Heap] %u KB dynamic buffer at %p\n",
            (unsigned)(HEAP_INITIAL_SIZE / 1024), (void *)s_head);
}

/* ------------------------------------------------------------------ */
/* Split block b into [size] + [remainder]                             */
/* ------------------------------------------------------------------ */
static void split(heap_block_t *b, size_t size) {
    if (b->size < size + sizeof(heap_block_t) + 8) return; /* not worth it */
    heap_block_t *new = (heap_block_t *)((uint8_t *)b + sizeof(heap_block_t) + size);
    new->magic = HEAP_MAGIC_FREE;
    new->size  = b->size - size - sizeof(heap_block_t);
    new->free  = true;
    new->next  = b->next;
    b->next    = new;
    b->size    = size;
}

/* ------------------------------------------------------------------ */
/* Coalesce adjacent free blocks                                        */
/* ------------------------------------------------------------------ */
static void coalesce(void) {
    heap_block_t *b = s_head;
    while (b && b->next) {
        if (b->free && b->next->free) {
            b->size += sizeof(heap_block_t) + b->next->size;
            b->next  = b->next->next;
        } else {
            b = b->next;
        }
    }
}

/* ------------------------------------------------------------------ */
/* kmalloc                                                              */
/* ------------------------------------------------------------------ */
void *kmalloc(size_t size) {
    if (!size) return NULL;
    size = ALIGN_UP(size, 8);   /* keep 8-byte alignment */

    heap_block_t *b = s_head;
    while (b) {
        if (b->free && b->size >= size) {
            split(b, size);
            b->free  = false;
            b->magic = HEAP_MAGIC_USED;
            return (void *)((uint8_t *)b + sizeof(heap_block_t));
        }
        b = b->next;
    }
    kpanic_color(0x0000AA00, 0x00000000, "kmalloc: out of kernel heap (requested %zu bytes)\n", size);
}

/* ------------------------------------------------------------------ */
/* kmalloc_aligned                                                      */
/* ------------------------------------------------------------------ */
void *kmalloc_aligned(size_t size, size_t alignment) {
    /* Simple strategy: over-allocate by alignment, then align pointer.
     * Wastes up to (alignment - 1) bytes but keeps things simple. */
    void *raw = kmalloc(size + alignment);
    uintptr_t addr = ALIGN_UP((uintptr_t)raw, alignment);
    return (void *)addr;
}

/* ------------------------------------------------------------------ */
/* kfree                                                                */
/* ------------------------------------------------------------------ */
void kfree(void *ptr) {
    if (!ptr) return;
    heap_block_t *b = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    if (b->magic != HEAP_MAGIC_USED) {
        kpanic("kfree: bad magic at %p (double free or corruption?)\n", ptr);
    }
    b->free  = true;
    b->magic = HEAP_MAGIC_FREE;
    coalesce();
}

/* ------------------------------------------------------------------ */
/* krealloc                                                             */
/* ------------------------------------------------------------------ */
void *krealloc(void *ptr, size_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (!new_size) { kfree(ptr); return NULL; }

    heap_block_t *b = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    if (b->size >= new_size) return ptr;    /* already big enough */

    void *new_ptr = kmalloc(new_size);
    kmemcpy(new_ptr, ptr, b->size);
    kfree(ptr);
    return new_ptr;
}
