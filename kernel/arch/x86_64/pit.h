/*
 * kernel/arch/x86_64/pit.h
 * PIT (Programmable Interval Timer 8253/8254) driver.
 */
#ifndef PIT_H
#define PIT_H

#include "../../include/kernel.h"

/* System ticks since boot */
extern volatile uint64_t g_pit_ticks;

void pit_init(uint32_t frequency_hz);
void pit_sleep(uint64_t ms);
uint64_t pit_get_ticks(void);

#endif /* PIT_H */
