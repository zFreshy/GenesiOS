/*
 * kernel/include/kernel.h
 * Core types, macros and inline helpers for the Genesi kernel.
 * Uses only compiler-provided freestanding headers.
 */
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

/* ------------------------------------------------------------------ */
/* Compiler attributes                                                 */
/* ------------------------------------------------------------------ */
#define PACKED      __attribute__((packed))
#define ALIGNED(x)  __attribute__((aligned(x)))
#define NORETURN    __attribute__((noreturn))
#define UNUSED      __attribute__((unused))
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

/* ------------------------------------------------------------------ */
/* x86-64 I/O port access                                             */
/* ------------------------------------------------------------------ */
ALWAYS_INLINE void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}
ALWAYS_INLINE uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port) : "memory");
    return val;
}
ALWAYS_INLINE void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}
ALWAYS_INLINE uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"(port) : "memory");
    return val;
}
/* Short I/O delay — write to unused port 0x80 */
ALWAYS_INLINE void io_wait(void) { outb(0x80, 0x00); }

/* ------------------------------------------------------------------ */
/* Interrupt control                                                   */
/* ------------------------------------------------------------------ */
ALWAYS_INLINE void irq_enable(void)  { __asm__ volatile ("sti" ::: "memory"); }
ALWAYS_INLINE void irq_disable(void) { __asm__ volatile ("cli" ::: "memory"); }
ALWAYS_INLINE void cpu_halt(void)    { __asm__ volatile ("hlt" ::: "memory"); }

/* Spin forever with interrupts disabled */
ALWAYS_INLINE NORETURN void panic_halt(void) {
    irq_disable();
    for (;;) cpu_halt();
}

/* ------------------------------------------------------------------ */
/* Memory helpers (no libc available yet)                              */
/* ------------------------------------------------------------------ */
static inline void *kmemset(void *dst, int val, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    while (n--) *p++ = (uint8_t)val;
    return dst;
}
static inline void *kmemcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;
    return dst;
}
static inline int kmemcmp(const void *a, const void *b, size_t n) {
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    while (n--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}
static inline size_t kstrlen(const char *s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}
static inline int kstrcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* ------------------------------------------------------------------ */
/* Bit manipulation helpers                                            */
/* ------------------------------------------------------------------ */
#define BIT(n)        (1ULL << (n))
#define ALIGN_UP(v,a) (((v) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(v,a) ((v) & ~((a) - 1))

#endif /* KERNEL_H */
