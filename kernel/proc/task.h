/*
 * kernel/proc/task.h
 * Process / thread descriptor (Fase 4).
 */
#ifndef TASK_H
#define TASK_H

#include "../include/kernel.h"

/* ------------------------------------------------------------------ */
/* Task states                                                         */
/* ------------------------------------------------------------------ */
typedef enum {
    TASK_RUNNING  = 0,
    TASK_READY    = 1,
    TASK_BLOCKED  = 2,
    TASK_ZOMBIE   = 3,
    TASK_DEAD     = 4,
} task_state_t;

/* ------------------------------------------------------------------ */
/* Default scheduling constants                                        */
/* ------------------------------------------------------------------ */
#define TASK_DEFAULT_TIMESLICE  10   /* PIT ticks (100 ms at 100 Hz)  */
#define TASK_KERNEL_STACK_SIZE  8192 /* 8 KB kernel stack per task     */
#define TASK_NAME_MAX           64
#define TASK_MAX_FDS            64

/* ------------------------------------------------------------------ */
/* Task control block                                                  */
/* ------------------------------------------------------------------ */
typedef struct task {
    uint64_t      pid;
    uint64_t      ppid;              /* parent PID */
    task_state_t  state;
    char          name[TASK_NAME_MAX];

    /* Scheduling */
    uint8_t       priority;          /* 0 = highest (reserved), 1..255 */
    uint32_t      timeslice;         /* remaining ticks before preempt */
    uint32_t      timeslice_reload;  /* reset value on reschedule      */

    /* CPU context — saved by switch_context */
    uint64_t      rsp;               /* kernel stack pointer (saved)   */
    uint64_t      rip;               /* entry point (for first switch) */
    uint64_t      cr3;               /* page table (per-process)       */

    /* Stack */
    uint8_t      *kernel_stack;      /* base of allocated stack        */
    size_t        kernel_stack_size;

    /* User Mode details */
    uint64_t      user_entry;
    uint64_t      user_rsp;

    /* Linked list for scheduler run queue */
    struct task  *next;
} task_t;

/* Entry-point signature for kernel threads */
typedef void (*task_entry_t)(void);

#endif /* TASK_H */
