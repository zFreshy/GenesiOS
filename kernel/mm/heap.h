/*
 * kernel/mm/heap.h
 * Kernel heap allocator (kmalloc / kfree).
 */
#ifndef HEAP_H
#define HEAP_H

#include "../include/kernel.h"

void  heap_init(void);
void *kmalloc(size_t size);
void *kmalloc_aligned(size_t size, size_t alignment);
void  kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

#endif /* HEAP_H */
