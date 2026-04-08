/*
 * kernel/include/kprintf.h
 * Minimal kernel printf — outputs to VGA text buffer.
 */
#ifndef KPRINTF_H
#define KPRINTF_H

#include "kernel.h"

/* Formatted kernel print — supports: %c %s %d %u %x %X %p %zu */
void kprintf(const char *fmt, ...);

/* Panic: print message + halt forever */
void kpanic(const char *fmt, ...) NORETURN;

#endif /* KPRINTF_H */
