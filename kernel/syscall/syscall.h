/*
 * kernel/syscall/syscall.h
 * System call interface (Fase 6).
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "../include/kernel.h"

/* Syscall numbers (Linux-compatible subset) */
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_EXIT    60
#define SYS_FORK    57
#define SYS_EXEC    59
#define SYS_MMAP    9
#define SYS_MUNMAP  11
#define SYS_GETPID  39

/* TODO: Phase 6 implementation */
void syscall_init(void);

#endif /* SYSCALL_H */
