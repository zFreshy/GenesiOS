/*
 * kernel/proc/task.h
 * Process / thread descriptor (Fase 4).
 */
#ifndef TASK_H
#define TASK_H

#include "../include/kernel.h"

typedef enum {
    TASK_RUNNING  = 0,
    TASK_READY    = 1,
    TASK_BLOCKED  = 2,
    TASK_DEAD     = 3,
} task_state_t;

typedef struct task {
    uint64_t pid;
    uint64_t ppid;              /* parent PID */
    task_state_t state;
    char     name[64];

    /* CPU context — saved on switch */
    uint64_t rsp;               /* kernel stack pointer */
    uint64_t rip;
    uint64_t cr3;               /* page table (per-process) */

    /* Stack */
    uint8_t *kernel_stack;
    size_t   kernel_stack_size;

    struct task *next;          /* scheduler linked list */
} task_t;

#endif /* TASK_H */
