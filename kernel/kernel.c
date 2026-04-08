/*
 * kernel/kernel.c
 * Genesi OS — Kernel entry point.
 *
 * Called from boot/boot.asm after the CPU enters 64-bit long mode.
 * Signature must match the call in long_mode_entry:
 *   void kernel_main(uint32_t boot_magic, uint64_t mboot_info)
 */
#include "include/kernel.h"
#include "include/multiboot2.h"
#include "include/vga.h"
#include "include/kprintf.h"

#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/isr.h"
#include "arch/x86_64/irq.h"
#include "arch/x86_64/pit.h"

#include "drivers/keyboard.h"

#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"

#include "proc/scheduler.h"

/* ------------------------------------------------------------------ */
/* Banner                                                               */
/* ------------------------------------------------------------------ */
static void print_banner(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    kprintf("   ____                    _\n");
    kprintf("  / ___| ___ _ __   ___  ___ (_)\n");
    kprintf(" | |  _ / _ \\ '_ \\ / _ \\/ __|| |\n");
    kprintf(" | |_| |  __/ | | |  __/\\__ \\| |\n");
    kprintf("  \\____|\___|_| |_|\\___|____/|_|\n");

    vga_set_color(VGA_DARK_GREY, VGA_BLACK);
    kprintf("      The Programming Operating System\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("\n");
}

/* ------------------------------------------------------------------ */
/* Status line helper                                                   */
/* ------------------------------------------------------------------ */
static void ok(const char *msg) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    kprintf("  [OK] ");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("%s\n", msg);
}

/* ------------------------------------------------------------------ */
/* Simple interactive REPL — echoes keys and runs basic commands        */
/* ------------------------------------------------------------------ */
static char s_line[256];

static void shell_prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    kprintf("genesi");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("> ");
}

/* ------------------------------------------------------------------ */
/* Test kernel threads for scheduler verification                      */
/* ------------------------------------------------------------------ */
static void test_thread_a(void) {
    for (int i = 0; i < 5; i++) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        kprintf("A");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        pit_sleep(500);
    }
    sched_exit();
}

static void test_thread_b(void) {
    for (int i = 0; i < 5; i++) {
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        kprintf("B");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        pit_sleep(500);
    }
    sched_exit();
}

static void test_thread_c(void) {
    for (int i = 0; i < 5; i++) {
        vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
        kprintf("C");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        pit_sleep(500);
    }
    sched_exit();
}

static int kstrcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static void shell_exec(const char *cmd) {
    if (!cmd[0]) return;

    if (kstrcmp(cmd, "help") == 0) {
        kprintf("  help      - show this help\n");
        kprintf("  mem       - show memory info\n");
        kprintf("  clear     - clear screen\n");
        kprintf("  version   - show OS info\n");
        kprintf("  test      - spawn 3 test threads (A, B, C)\n");
        kprintf("  halt      - halt the system\n");
    } else if (kstrcmp(cmd, "mem") == 0) {
        kprintf("  Free  : %zu MB\n",
                (size_t)(pmm_free_frames()  * PAGE_SIZE / (1024*1024)));
        kprintf("  Total : %zu MB\n",
                (size_t)(pmm_total_frames() * PAGE_SIZE / (1024*1024)));
    } else if (kstrcmp(cmd, "clear") == 0) {
        vga_clear();
    } else if (kstrcmp(cmd, "version") == 0) {
        kprintf("  Genesi OS v0.2-dev (x86-64, C + Assembly)\n");
        kprintf("  Uptime: %zu ticks\n", (size_t)pit_get_ticks());
    } else if (kstrcmp(cmd, "test") == 0) {
        kprintf("  Spawning 3 test threads...\n");
        task_t *a = sched_create_task("test-A", test_thread_a);
        task_t *b = sched_create_task("test-B", test_thread_b);
        task_t *c = sched_create_task("test-C", test_thread_c);
        sched_add(a);
        sched_add(b);
        sched_add(c);
        kprintf("  Threads created. Watch for interleaved A/B/C output.\n");
    } else if (kstrcmp(cmd, "halt") == 0) {
        kprintf("  Halting...\n");
        irq_disable();
        for (;;) cpu_halt();
    } else {
        kprintf("  Unknown command: '%s'  (type 'help')\n", cmd);
    }
}

static void run_shell(void) {
    kprintf("\n");
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    kprintf("  Genesi Shell — type 'help' for commands.\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("\n");

    while (true) {
        shell_prompt();
        size_t len = 0;
        while (true) {
            char c = keyboard_getchar();
            if (c == '\n' || c == '\r') {
                vga_putchar('\n');
                s_line[len] = '\0';
                break;
            }
            if (c == '\b' && len > 0) {
                len--;
                vga_putchar('\b');
                continue;
            }
            if (len < sizeof(s_line) - 1) {
                s_line[len++] = c;
                vga_putchar(c);
            }
        }
        shell_exec(s_line);
    }
}

/* ------------------------------------------------------------------ */
/* kernel_main                                                          */
/* ------------------------------------------------------------------ */
void kernel_main(uint32_t boot_magic, uint64_t mboot_info) {
    /* --- Phase 1: VGA + banner ------------------------------------ */
    vga_init();
    print_banner();

    if (boot_magic != MB2_BOOTLOADER_MAGIC) {
        kpanic("Not booted by a Multiboot2 bootloader! (magic=0x%x)\n",
               boot_magic);
    }

    /* --- Phase 2: CPU architecture -------------------------------- */
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    kprintf("  Initializing hardware...\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);

    gdt_init();       ok("GDT loaded (null/kcode/kdata/ucode/udata + TSS)");
    idt_init();       ok("IDT installed (256 interrupt gates)");
    irq_init();       ok("PIC 8259 remapped (IRQ0-7 -> vec 32-39)");
    pit_init(100);    ok("PIT timer initialized at 100 Hz");
    keyboard_init();  ok("PS/2 keyboard driver active");

    /* --- Phase 3: Memory ------------------------------------------ */
    kprintf("\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    kprintf("  Initializing memory...\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);

    pmm_init(mboot_info);  ok("Physical Memory Manager ready");
    vmm_init();            ok("Virtual Memory Manager (4-level paging)");
    heap_init();           ok("Kernel heap allocator ready");

    /* --- Phase 4: Scheduler --------------------------------------- */
    sched_init();          ok("Preemptive scheduler active (PID 0: idle, PID 1: kernel)");

    /* --- Enable interrupts ---------------------------------------- */
    irq_enable();
    kprintf("\n");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    kprintf("  System ready. ");
    vga_set_color(VGA_DARK_GREY, VGA_BLACK);
    kprintf("(%zu MB RAM available)\n",
            (size_t)(pmm_free_frames() * PAGE_SIZE / (1024 * 1024)));
    vga_set_color(VGA_WHITE, VGA_BLACK);

    /* --- Phase 7 preview: basic interactive shell ----------------- */
    run_shell();

    /* Should never reach here */
    panic_halt();
}
