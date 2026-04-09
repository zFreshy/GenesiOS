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
#include "include/net.h"

#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/isr.h"
#include "arch/x86_64/irq.h"
#include "arch/x86_64/pit.h"

#include "drivers/keyboard.h"
#include "drivers/mouse.h"

#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"

#include "gfx/framebuffer.h"
#include "gfx/fb_console.h"
#include "gui/desktop.h"
#include "gui/compositor.h"

#include "proc/scheduler.h"
#include "syscall/syscall.h"

/* ------------------------------------------------------------------ */
/* Banner                                                               */
/* ------------------------------------------------------------------ */
static void print_banner(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    kprintf("   ____                    _\n");
    kprintf("  / ___| ___ _ __   ___  ___ _ (_)\n");
    kprintf(" | |  _ / _ \\ '_ \\ / _ \\/ __| |\n");
    kprintf(" | |_| |  __/ | | |  __/\\__ \\| |\n");
    kprintf("  \\____|\\___|_| |_|\\___|____/|_|\n");

    vga_set_color(VGA_DARK_GREY, VGA_BLACK);
    kprintf("      The Programming Operating System\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("\n");
}

/* ------------------------------------------------------------------ */
/* Status line helper                                                   */
/* ------------------------------------------------------------------ */
static void ok(const char *msg) {
    if (fb_available()) { fbc_set_fg(FBC_LIGHT_GREEN); }
    else vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    kprintf("  [OK] ");
    if (fb_available()) { fbc_set_fg(FBC_WHITE); }
    else vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("%s\n", msg);
}

/* ------------------------------------------------------------------ */
/* Simple interactive REPL — echoes keys and runs basic commands        */
/* ------------------------------------------------------------------ */
static char s_line[256];

static void shell_prompt(void) {
    if (fb_available()) fbc_set_fg(FBC_LIGHT_GREEN);
    else vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    kprintf("genesi");
    if (fb_available()) fbc_set_fg(FBC_WHITE);
    else vga_set_color(VGA_WHITE, VGA_BLACK);
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

// We already have kstrcmp in kernel.h

/* Mouse IRQ wrapper */
static void mouse_irq_wrapper(registers_t *regs) {
    (void)regs;
    mouse_irq_handler();
    /* Update GUI when mouse moves */
    if (fb_available()) compositor_update();
    irq_send_eoi(IRQ_MOUSE);
}

uint64_t g_mboot_info = 0;

void shell_exec(const char *cmd) {
    if (!cmd[0]) return;

    if (kstrcmp(cmd, "help") == 0) {
        kprintf("  help      - show this help\n");
        kprintf("  mem       - show memory info\n");
        kprintf("  clear     - clear screen\n");
        kprintf("  version   - show OS info\n");
        kprintf("  test      - spawn 3 test threads (A, B, C)\n");
        kprintf("  ipconfig  - show network interfaces\n");
        kprintf("  nettest   - test E1000 sending a raw packet\n");
        kprintf("  halt      - halt the system\n");
    } else if (kstrcmp(cmd, "mem") == 0) {
        kprintf("  Free  : %zu MB\n",
                (size_t)(pmm_free_frames()  * PAGE_SIZE / (1024*1024)));
        kprintf("  Total : %zu MB\n",
                (size_t)(pmm_total_frames() * PAGE_SIZE / (1024*1024)));
    } else if (kstrcmp(cmd, "clear") == 0) {
        if (fb_available()) fbc_clear();
        else vga_clear();
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
    } else if (kstrcmp(cmd, "ipconfig") == 0) {
        extern net_device_t g_net_dev;
        if (!g_net_dev.present) {
            kprintf("\n  Windows IP Configuration\n\n");
            kprintf("  No network interfaces found.\n");
        } else {
            kprintf("\n  Windows IP Configuration\n\n");
            kprintf("  Ethernet adapter Local Area Connection:\n\n");
            kprintf("     Connection-specific DNS Suffix  . : genesi.local\n");
            kprintf("     Description . . . . . . . . . . . : %s\n", g_net_dev.name);
            kprintf("     Physical Address. . . . . . . . . : %X-%X-%X-%X-%X-%X\n", 
                    g_net_dev.mac[0], g_net_dev.mac[1], g_net_dev.mac[2], 
                    g_net_dev.mac[3], g_net_dev.mac[4], g_net_dev.mac[5]);
            
            if (g_net_dev.ip[0] == 0) {
                kprintf("     DHCP Enabled. . . . . . . . . . . : Yes\n");
                kprintf("     IPv4 Address. . . . . . . . . . . : (Connecting...)\n");
            } else {
                kprintf("     IPv4 Address. . . . . . . . . . . : %d.%d.%d.%d\n", 
                        g_net_dev.ip[0], g_net_dev.ip[1], g_net_dev.ip[2], g_net_dev.ip[3]);
                kprintf("     Subnet Mask . . . . . . . . . . . : %d.%d.%d.%d\n", 
                        g_net_dev.mask[0], g_net_dev.mask[1], g_net_dev.mask[2], g_net_dev.mask[3]);
                kprintf("     Default Gateway . . . . . . . . . : %d.%d.%d.%d\n", 
                        g_net_dev.gateway[0], g_net_dev.gateway[1], g_net_dev.gateway[2], g_net_dev.gateway[3]);
            }
        }
        kprintf("\n");
    } else if (kstrcmp(cmd, "nettest") == 0) {
        extern net_device_t g_net_dev;
        if (!g_net_dev.present) {
            kprintf("  Error: No network device available to test.\n");
        } else {
            kprintf("  [NetTest] Preparing test packet...\n");
            
            /* Basic raw Ethernet frame for testing (Broadcast ping basically) */
            uint8_t test_packet[64];
            kmemset(test_packet, 0, sizeof(test_packet));
            
            /* Destination MAC (Broadcast) */
            for (int i = 0; i < 6; i++) test_packet[i] = 0xFF;
            /* Source MAC */
            for (int i = 0; i < 6; i++) test_packet[6+i] = g_net_dev.mac[i];
            /* EtherType (IPv4 = 0x0800) */
            test_packet[12] = 0x08; test_packet[13] = 0x00;
            
            /* Fake payload */
            const char* payload = "GENESI_NET_TEST_PACKET";
            for (int i = 0; payload[i] && i < 40; i++) {
                test_packet[14+i] = payload[i];
            }
            
            kprintf("  [NetTest] Sending 64-byte raw frame via E1000...\n");
            extern int e1000_send_packet(const void *data, uint16_t len);
            int res = e1000_send_packet(test_packet, 64);
            
            if (res == 0) {
                kprintf("  [NetTest] SUCCESS! Packet sent to the wire (DD bit set).\n");
            } else {
                kprintf("  [NetTest] FAILED! e1000_send_packet returned %d.\n", res);
            }
        }
    } else if (kstrcmp(cmd, "run") == 0) {
        kprintf("  Running test user process...\n");
        extern void process_create_user(const char *name, const uint8_t *elf_data);
        
        mb2_info_t *info = (mb2_info_t *)(uintptr_t)g_mboot_info;
        mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)info + 8);
        
        while (tag->type != MB2_TAG_END) {
            if (tag->type == 3) { // MB2_TAG_MODULE
                uint32_t mod_start = *(uint32_t *)((uint8_t *)tag + 8);
                uint8_t *mod_data = (uint8_t *)(uintptr_t)mod_start;
                // Only execute if it is an ELF file (Magic: 0x7F 'E' 'L' 'F')
                if (mod_data[0] == 0x7F && mod_data[1] == 'E' && mod_data[2] == 'L' && mod_data[3] == 'F') {
                    process_create_user("test", mod_data);
                    return;
                }
            }
            tag = (mb2_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
        }
        kprintf("  Error: Could not find an ELF module in Multiboot info.\n");
    } else if (kstrcmp(cmd, "halt") == 0) {
        kprintf("  Halting...\n");
        irq_disable();
        for (;;) cpu_halt();
    } else {
        kprintf("  Unknown command: '%s'  (type 'help')\n", cmd);
    }
}

static void __attribute__((unused)) run_shell(void) {
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
                if (fb_available()) fbc_putchar('\n');
                else vga_putchar('\n');
                s_line[len] = '\0';
                break;
            }
            if (c == '\b' && len > 0) {
                len--;
                /* Deleting a char in FB console needs some logic or just print space over it */
                if (fb_available()) {
                    /* Basic backspace */
                    fbc_putchar('\b'); 
                    fbc_putchar(' ');
                    fbc_putchar('\b');
                } else {
                    vga_putchar('\b');
                }
                continue;
            }
            if (len < sizeof(s_line) - 1) {
                s_line[len++] = c;
                if (fb_available()) fbc_putchar(c);
                else vga_putchar(c);
            }
        }
        shell_exec(s_line);
    }
}

/* ------------------------------------------------------------------ */
/* Shutdown / Power Off                                               */
/* ------------------------------------------------------------------ */
void system_shutdown(void) {
    kprintf("\n[ACPI] Shutting down virtual machine...\n");
    
    /* Desabilita interrupções pra não ter tela azul no meio do desligamento */
    irq_disable();
    
    /* Portas mágicas ACPI para emuladores comuns */
    outw(0x604, 0x2000);  /* QEMU newer than 2.0 */
    outw(0xB004, 0x2000); /* Older QEMU / Bochs */
    outw(0x4004, 0x3400); /* VirtualBox */
    
    /* Se nada disso funcionar (PC Real sem ACPI configurado), trava o CPU */
    for (;;) {
        cpu_halt();
    }
}

/* ------------------------------------------------------------------ */
/* kernel_main                                                        */
/* ------------------------------------------------------------------ */
void kernel_main(uint32_t boot_magic, uint64_t mboot_info) {
    g_mboot_info = mboot_info;

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
    mouse_init();     ok("PS/2 mouse driver active");
    irq_register_handler(IRQ_MOUSE, mouse_irq_wrapper);
    irq_unmask(IRQ_MOUSE);

    /* --- Phase 3: Memory ------------------------------------------ */
    kprintf("\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    kprintf("  Initializing memory...\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);

    pmm_init(mboot_info);  ok("Physical Memory Manager ready");
    vmm_init();            ok("Virtual Memory Manager (4-level paging)");
    heap_init();           ok("Kernel heap allocator ready");

    /* --- Phase 8: Framebuffer ---------------------------------------- */
    fb_init(mboot_info);
    extern void font_init(uint64_t);
    font_init(mboot_info);
    fb_console_init();
    kprintf_enable_fb();

    /* Reprint the banner now that we're in graphical mode */
    kprintf("\n   ____                    _\n");
    kprintf("  / ___| ___ _ __   ___  ___ (_)\n");
    kprintf(" | |  _ / _ \\ '_ \\/ _ \\/ __|| |\n");
    kprintf(" | |_| |  __/ | | |  __/\\__ \\| |\n");
    kprintf("  \\____|\\___| | |_|\\___||____/|_|\n");
    kprintf("      The Programming Operating System\n\n");
    ok("Framebuffer console active");

    /* --- Phase 9: PCI & Devices ----------------------------------- */
    #include "include/pci.h"
    pci_init();

    /* --- Phase 4: Scheduler --------------------------------------- */
    sched_init();          ok("Scheduler initialized (PID 0 idle, PID 1 kernel)");

    /* --- Phase 5: Syscalls ---------------------------------------- */
    syscall_init();        ok("Syscalls enabled (SYSCALL/SYSRET)");

    /* --- Enable interrupts ---------------------------------------- */
    irq_enable();
    kprintf("\n");
    kprintf("  System ready. ");
    kprintf("(%zu MB RAM available)\n",
            (size_t)(pmm_free_frames() * PAGE_SIZE / (1024 * 1024)));

    kprintf("  Loading Desktop Environment...\n");

    /* --- Phase 10: Desktop & Window Manager -------------------------- */
    desktop_start();

    /* Enter idle loop for the GUI */
    for (;;) {
        if (keyboard_has_char()) {
            char c = keyboard_getchar();
            window_t *top = wm_get_top();
            if (top && top->on_key) {
                top->on_key(top, c);
            }
        } else {
            cpu_halt();
        }
    }
}
