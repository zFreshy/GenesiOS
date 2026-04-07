/*
 * kernel/proc/scheduler.c
 * Stub — full implementation in Fase 4.
 */
#include "scheduler.h"
#include "../include/kernel.h"

static task_t *s_current = NULL;

void    sched_init(void)          { /* TODO: Phase 4 */ }
void    sched_add(task_t *t)      { s_current = t; }
void    sched_remove(task_t *t)   { (void)t; }
void    sched_yield(void)         { }
task_t *sched_current(void)       { return s_current; }
void    sched_tick(void)          { /* TODO: context switch */ }
