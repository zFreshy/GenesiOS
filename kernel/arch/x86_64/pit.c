/*
 * kernel/arch/x86_64/pit.c
 * PIT channel 0 in rate-generator mode.
 * Base oscillator: 1,193,182 Hz
 */
#include "pit.h"
#include "irq.h"
#include "../../include/kernel.h"
#include "../../proc/scheduler.h"

#define PIT_CHANNEL0  0x40
#define PIT_CMD       0x43
#define PIT_BASE_HZ   1193182UL

volatile uint64_t g_pit_ticks = 0;

static uint32_t s_hz = 0;

/* ------------------------------------------------------------------ */
/* IRQ0 handler                                                        */
/* ------------------------------------------------------------------ */
static void pit_irq_handler(registers_t *regs) {
    (void)regs;
    g_pit_ticks++;
    irq_send_eoi(IRQ_TIMER);
    sched_tick();
}

/* ------------------------------------------------------------------ */
/* pit_init                                                             */
/* ------------------------------------------------------------------ */
void pit_init(uint32_t frequency_hz) {
    s_hz = frequency_hz;

    uint32_t divisor = (uint32_t)(PIT_BASE_HZ / frequency_hz);

    /* Channel 0, lo/hi byte access, rate generator (mode 2), binary */
    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    irq_register_handler(IRQ_TIMER, pit_irq_handler);
    irq_unmask(IRQ_TIMER);
}

/* ------------------------------------------------------------------ */
/* pit_get_ticks                                                        */
/* ------------------------------------------------------------------ */
uint64_t pit_get_ticks(void) {
    return g_pit_ticks;
}

/* ------------------------------------------------------------------ */
/* pit_sleep — busy-wait for ms milliseconds                           */
/* ------------------------------------------------------------------ */
void pit_sleep(uint64_t ms) {
    uint64_t target = g_pit_ticks + (ms * s_hz / 1000);
    while (g_pit_ticks < target) {
        __asm__ volatile ("pause");
    }
}
