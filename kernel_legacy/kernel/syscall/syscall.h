/*
 * kernel/syscall/syscall.h
 * Syscall declarations and definitions.
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "../include/kernel.h"

/* Syscall numbers */
#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_GETPID  3
#define SYS_EXEC    4
#define SYS_MMAP    5

/* Register MSRs for SYSCALL/SYSRET */
#define MSR_EFER   0xC0000080
#define MSR_STAR   0xC0000081
#define MSR_LSTAR  0xC0000082
#define MSR_CSTAR  0xC0000083
#define MSR_SFMASK 0xC0000084

void syscall_init(void);

/* Called from assembly */
uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);

#endif /* SYSCALL_H */