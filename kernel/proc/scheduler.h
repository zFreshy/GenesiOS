/*
 * kernel/proc/scheduler.h
 * Round-Robin preemptive scheduler interface (Fase 4).
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

/* ------------------------------------------------------------------ */
/* Lifecycle                                                           */
/* ------------------------------------------------------------------ */
void     sched_init(void);

/* ------------------------------------------------------------------ */
/* Task management                                                     */
/* ------------------------------------------------------------------ */
task_t  *sched_create_task(const char *name, task_entry_t entry);
void     process_create_user(const char *name, const uint8_t *elf_data);
void     sched_add(task_t *task);
void     sched_remove(task_t *task);
void     sched_exit(void);            /* terminate current task        */

/* ------------------------------------------------------------------ */
/* Scheduling control                                                  */
/* ------------------------------------------------------------------ */
void     sched_yield(void);           /* voluntary preemption          */
void     sched_block(task_t *task);   /* move to BLOCKED state         */
void     sched_unblock(task_t *task); /* move back to READY            */
task_t  *sched_current(void);         /* currently running task        */
void     sched_tick(void);            /* called from PIT IRQ0 handler  */

/* Context switch — implemented in context_switch.asm */
extern void switch_context(uint64_t *old_rsp, uint64_t new_rsp);

#endif /* SCHEDULER_H */
