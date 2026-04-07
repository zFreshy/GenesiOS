/*
 * kernel/proc/scheduler.h
 * Round-Robin scheduler interface (Fase 4).
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

void     sched_init(void);
void     sched_add(task_t *task);
void     sched_remove(task_t *task);
void     sched_yield(void);        /* voluntary preemption */
task_t  *sched_current(void);      /* currently running task */
void     sched_tick(void);         /* called from PIT IRQ0 handler */

#endif /* SCHEDULER_H */
