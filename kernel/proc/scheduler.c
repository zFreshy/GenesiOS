/*
 * kernel/proc/scheduler.c
 * Round-Robin preemptive scheduler (Fase 4).
 *
 * Tasks are kept in a circular singly-linked list.
 * PID 0 = idle task (HLT loop when nothing else is runnable).
 * PID 1 = kernel task (represents the boot thread / shell).
 */
#include "scheduler.h"
#include "../include/kernel.h"
#include "../include/kprintf.h"
#include "../mm/heap.h"

/* ------------------------------------------------------------------ */
/* Globals                                                              */
/* ------------------------------------------------------------------ */
static task_t *s_run_queue  = NULL;   /* circular list head            */
static task_t *s_current    = NULL;   /* currently running task        */
static uint64_t s_next_pid  = 0;

/* ------------------------------------------------------------------ */
/* Idle task — runs when no other task is READY                         */
/* ------------------------------------------------------------------ */
static void idle_task_entry(void) {
    for (;;) {
        __asm__ volatile ("sti; hlt" ::: "memory");
    }
}

/* ------------------------------------------------------------------ */
/* task_trampoline — wrapper so tasks can return cleanly                */
/* ------------------------------------------------------------------ */
static void task_trampoline(void) {
    /* The real entry point was pushed onto the stack by sched_create_task
     * and is now in R15 (restored by switch_context). We call it. */

    /* We get here via ret from switch_context. The entry point address
     * is in R15 which was set up in create. We need a small asm trick. */
    task_entry_t entry;
    __asm__ volatile ("mov %%r15, %0" : "=r"(entry));

    irq_enable();
    entry();

    /* If the task returns, terminate it */
    sched_exit();
}

/* ------------------------------------------------------------------ */
/* sched_create_task — allocate and initialize a new task               */
/* ------------------------------------------------------------------ */
task_t *sched_create_task(const char *name, task_entry_t entry) {
    task_t *t = (task_t *)kmalloc(sizeof(task_t));
    if (!t) {
        kprintf("[SCHED] ERROR: failed to allocate task_t\n");
        return NULL;
    }
    kmemset(t, 0, sizeof(task_t));

    t->pid   = s_next_pid++;
    t->state = TASK_READY;
    t->priority = 1;
    t->timeslice = TASK_DEFAULT_TIMESLICE;
    t->timeslice_reload = TASK_DEFAULT_TIMESLICE;

    /* Copy name */
    size_t nlen = kstrlen(name);
    if (nlen >= TASK_NAME_MAX) nlen = TASK_NAME_MAX - 1;
    kmemcpy(t->name, name, nlen);
    t->name[nlen] = '\0';

    /* Allocate kernel stack */
    t->kernel_stack_size = TASK_KERNEL_STACK_SIZE;
    t->kernel_stack = (uint8_t *)kmalloc(TASK_KERNEL_STACK_SIZE);
    if (!t->kernel_stack) {
        kprintf("[SCHED] ERROR: failed to allocate kernel stack\n");
        kfree(t);
        return NULL;
    }
    kmemset(t->kernel_stack, 0, TASK_KERNEL_STACK_SIZE);

    /* Use current CR3 (all kernel tasks share the same address space) */
    __asm__ volatile ("mov %%cr3, %0" : "=r"(t->cr3));

    /*
     * Set up the initial stack to look like switch_context just saved
     * everything. switch_context expects on the stack (top to bottom):
     *   [return address]  <- where ret goes
     *   [rbp]
     *   [rbx]
     *   [r12]
     *   [r13]
     *   [r14]
     *   [r15]             <- we store entry point here
     *   [cr3]
     *
     * We point RSP at the "cr3" slot so switch_context will pop them
     * in order and ret to task_trampoline, which reads R15 for entry.
     */
    uint64_t *stack_top = (uint64_t *)(t->kernel_stack + TASK_KERNEL_STACK_SIZE);

    /* Align stack to 16 bytes */
    stack_top = (uint64_t *)((uint64_t)stack_top & ~0xFULL);

    *(--stack_top) = (uint64_t)task_trampoline;  /* return address     */
    *(--stack_top) = 0;                          /* rbp                */
    *(--stack_top) = 0;                          /* rbx                */
    *(--stack_top) = 0;                          /* r12                */
    *(--stack_top) = 0;                          /* r13                */
    *(--stack_top) = 0;                          /* r14                */
    *(--stack_top) = (uint64_t)entry;            /* r15 = entry point  */
    *(--stack_top) = t->cr3;                     /* cr3                */

    t->rsp = (uint64_t)stack_top;

    return t;
}

/* ------------------------------------------------------------------ */
/* sched_add — insert task into the circular run queue                  */
/* ------------------------------------------------------------------ */
void sched_add(task_t *t) {
    if (!t) return;

    irq_disable();
    if (!s_run_queue) {
        t->next = t;          /* single-element circular list */
        s_run_queue = t;
    } else {
        /* Insert after current head */
        t->next = s_run_queue->next;
        s_run_queue->next = t;
    }
    irq_enable();
}

/* ------------------------------------------------------------------ */
/* sched_remove — remove task from run queue                            */
/* ------------------------------------------------------------------ */
void sched_remove(task_t *t) {
    if (!t || !s_run_queue) return;

    irq_disable();

    /* Single element? */
    if (t->next == t) {
        s_run_queue = NULL;
        irq_enable();
        return;
    }

    /* Find predecessor */
    task_t *prev = s_run_queue;
    while (prev->next != t) {
        prev = prev->next;
        if (prev == s_run_queue) {
            irq_enable();
            return; /* not found */
        }
    }
    prev->next = t->next;

    if (s_run_queue == t)
        s_run_queue = t->next;

    irq_enable();
}

/* ------------------------------------------------------------------ */
/* sched_current                                                        */
/* ------------------------------------------------------------------ */
task_t *sched_current(void) {
    return s_current;
}

/* ------------------------------------------------------------------ */
/* schedule — pick next READY task and switch to it                     */
/* ------------------------------------------------------------------ */
static void schedule(void) {
    if (!s_current || !s_run_queue) return;

    task_t *next = s_current->next;
    task_t *start = next;

    /* Find next READY or RUNNING task */
    while (next->state != TASK_READY && next->state != TASK_RUNNING) {
        next = next->next;
        if (next == start) return;  /* no runnable task (idle will run) */
    }

    if (next == s_current) return;  /* same task, no switch needed */

    task_t *prev = s_current;

    /* Mark states */
    if (prev->state == TASK_RUNNING)
        prev->state = TASK_READY;

    next->state = TASK_RUNNING;
    next->timeslice = next->timeslice_reload;
    s_current = next;

    switch_context(&prev->rsp, next->rsp);
}

/* ------------------------------------------------------------------ */
/* sched_tick — called from PIT IRQ0 handler                            */
/* ------------------------------------------------------------------ */
void sched_tick(void) {
    if (!s_current) return;

    if (s_current->timeslice > 0)
        s_current->timeslice--;

    if (s_current->timeslice == 0) {
        schedule();
    }
}

/* ------------------------------------------------------------------ */
/* sched_yield — voluntary preemption                                   */
/* ------------------------------------------------------------------ */
void sched_yield(void) {
    irq_disable();
    s_current->timeslice = 0;
    schedule();
    irq_enable();
}

/* ------------------------------------------------------------------ */
/* sched_block                                                          */
/* ------------------------------------------------------------------ */
void sched_block(task_t *task) {
    irq_disable();
    task->state = TASK_BLOCKED;
    if (task == s_current)
        schedule();
    irq_enable();
}

/* ------------------------------------------------------------------ */
/* sched_unblock                                                        */
/* ------------------------------------------------------------------ */
void sched_unblock(task_t *task) {
    task->state = TASK_READY;
    task->timeslice = task->timeslice_reload;
}

/* ------------------------------------------------------------------ */
/* sched_exit — terminate the current task                              */
/* ------------------------------------------------------------------ */
void sched_exit(void) {
    irq_disable();
    s_current->state = TASK_DEAD;
    schedule();
    /* Should never return here */
    for (;;) cpu_halt();
}

/* ------------------------------------------------------------------ */
/* sched_init — create idle (PID 0) and adopt boot thread as PID 1     */
/* ------------------------------------------------------------------ */
void sched_init(void) {
    /* PID 0: idle task */
    task_t *idle = sched_create_task("idle", idle_task_entry);
    idle->priority = 255;  /* lowest priority */
    sched_add(idle);

    /*
     * PID 1: the current boot thread (kernel_main).
     * We create a task_t for it but DON'T set up a fake stack —
     * it's already running, so switch_context will save its real
     * state when the first preemption happens.
     */
    task_t *kernel = (task_t *)kmalloc(sizeof(task_t));
    kmemset(kernel, 0, sizeof(task_t));
    kernel->pid   = s_next_pid++;
    kernel->state = TASK_RUNNING;
    kernel->timeslice = TASK_DEFAULT_TIMESLICE;
    kernel->timeslice_reload = TASK_DEFAULT_TIMESLICE;
    kernel->priority = 1;
    kmemcpy(kernel->name, "kernel", 7);
    __asm__ volatile ("mov %%cr3, %0" : "=r"(kernel->cr3));

    sched_add(kernel);
    s_current = kernel;
}
