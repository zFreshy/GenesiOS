/*
 * kernel/syscall/syscall.c
 * System call dispatch table and basic handlers.
 */
#include "syscall.h"
#include "../include/kprintf.h"
#include "../proc/scheduler.h"

/* The assembly entry point */
extern void syscall_entry(void);

/* Helpers to read/write MSRs */
static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = (uint32_t)val;
    uint32_t high = (uint32_t)(val >> 32);
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high) : "memory");
}

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
    return ((uint64_t)high << 32) | low;
}

/* ------------------------------------------------------------------ */
/* Handlers                                                             */
/* ------------------------------------------------------------------ */
static uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t count) {
    if (fd != 1 && fd != 2) return (uint64_t)-1; // Only stdout/stderr for now
    const char *str = (const char *)(uintptr_t)buf;
    for (uint64_t i = 0; i < count; i++) {
        kprintf("%c", str[i]);
    }
    return count;
}

static uint64_t sys_getpid(void) {
    task_t *t = sched_current();
    if (t) return t->pid;
    return 0;
}

static uint64_t sys_exit(uint64_t status) {
    kprintf("\n[SYSCALL] Process exited with status %llu\n", status);
    sched_exit();
    return 0; // Never reached
}

/* ------------------------------------------------------------------ */
/* syscall_init                                                         */
/* ------------------------------------------------------------------ */
void syscall_init(void) {
    /* Enable SCE (Syscall Enable) in EFER */
    uint64_t efer = rdmsr(MSR_EFER);
    wrmsr(MSR_EFER, efer | 1);

    /* 
     * Set up STAR (Segment Selector Register)
     * Bits 32-47: Kernel CS/SS base (CS = STAR[47:32], SS = STAR[47:32] + 8) -> CS=0x08, SS=0x10
     * Bits 48-63: User CS/SS base (CS = STAR[63:48] + 16, SS = STAR[63:48] + 8) -> CS=0x20, SS=0x18
     * Thus, we set STAR[47:32] = 0x08, STAR[63:48] = 0x10.
     */
    uint64_t star = ((0x10ULL) << 48) | ((0x08ULL) << 32);
    wrmsr(MSR_STAR, star);

    /* Set up LSTAR (Long Mode Syscall Target Address Register) */
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);

    /* Set up SFMASK (Syscall Flag Mask) - disable interrupts during syscall */
    wrmsr(MSR_SFMASK, 0x200); /* Disable interrupts (IF bit) */
}

/* ------------------------------------------------------------------ */
/* syscall_handler (called from ASM)                                   */
/* ------------------------------------------------------------------ */
uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a4; (void)a5;
    
    switch (num) {
        case SYS_EXIT:   return sys_exit(a1);
        case SYS_WRITE:  return sys_write(a1, a2, a3);
        case SYS_GETPID: return sys_getpid();
        
        default:
            kprintf("[SYSCALL] Unknown syscall: %llu\n", num);
            return (uint64_t)-1;
    }
}
