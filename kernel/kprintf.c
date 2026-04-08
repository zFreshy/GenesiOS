/*
 * kernel/kprintf.c
 * Kernel printf — dual output to VGA text (early boot) and fb_console (after fb_init).
 * Supported formats: %c %s %d %i %u %x %X %p %zu %llu %llx %%
 */
#include "include/kprintf.h"
#include "include/vga.h"
#include "gfx/fb_console.h"
#include "gfx/framebuffer.h"

/* Simple spinlock for kprintf if we have multiple CPUs/threads */
static bool s_kprintf_lock = false;
static bool s_fb_ready = false;

void kprintf_enable_fb(void) {
    s_fb_ready = true;
}

/* ------------------------------------------------------------------ */
/* Output one character to whichever backend is active                 */
/* ------------------------------------------------------------------ */
static void putc_backend(char c) {
    /* Log to COM1 serial port (0x3F8) for debugging */
    /* Check if the serial port is ready to transmit before writing to avoid blocking on real hardware */
    int timeout = 1000;
    while ((inb(0x3FD) & 0x20) == 0 && timeout > 0) {
        timeout--;
    }
    if (timeout > 0) {
        outb(0x3F8, c);
    }

    if (s_fb_ready) {
        fb_console_putchar(c);
    } else {
        vga_putchar(c);
    }
}

static void puts_backend(const char *s) {
    if (!s) s = "(null)";
    while (*s) putc_backend(*s++);
}

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */
static void print_uint(uint64_t v, int base, bool upper) {
    static const char lo[] = "0123456789abcdef";
    static const char hi[] = "0123456789ABCDEF";
    const char *digits = upper ? hi : lo;
    char buf[65]; int len = 0;
    if (v == 0) { putc_backend('0'); return; }
    while (v) { buf[len++] = digits[v % (uint64_t)base]; v /= (uint64_t)base; }
    for (int i = len - 1; i >= 0; i--) putc_backend(buf[i]);
}

static void print_int(int64_t v) {
    if (v < 0) { putc_backend('-'); v = -v; }
    print_uint((uint64_t)v, 10, false);
}

/* ------------------------------------------------------------------ */
/* vkprintf                                                            */
/* ------------------------------------------------------------------ */
void vkprintf(const char *fmt, va_list ap) {
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { putc_backend(*p); continue; }
        p++;
        switch (*p) {
        case 'c':
            putc_backend((char)va_arg(ap, int));
            break;
        case 's':
            puts_backend(va_arg(ap, const char *));
            break;
        case 'd': case 'i':
            print_int((int64_t)va_arg(ap, int));
            break;
        case 'u':
            print_uint((uint64_t)va_arg(ap, unsigned int), 10, false);
            break;
        case 'x':
            print_uint((uint64_t)va_arg(ap, unsigned int), 16, false);
            break;
        case 'X':
            print_uint((uint64_t)va_arg(ap, unsigned int), 16, true);
            break;
        case 'p':
            puts_backend("0x");
            print_uint((uint64_t)(uintptr_t)va_arg(ap, void *), 16, false);
            break;
        case 'z':
            if (*(p+1) == 'u') { p++;
                print_uint((uint64_t)va_arg(ap, size_t), 10, false);
            }
            break;
        case 'l':
            if (*(p+1) == 'l' && *(p+2) == 'u') { p += 2;
                print_uint(va_arg(ap, unsigned long long), 10, false);
            } else if (*(p+1) == 'l' && *(p+2) == 'x') { p += 2;
                print_uint(va_arg(ap, unsigned long long), 16, false);
            } else if (*(p+1) == 'u') { p++;
                print_uint((uint64_t)va_arg(ap, unsigned long), 10, false);
            }
            break;
        case '%':
            putc_backend('%');
            break;
        default:
            putc_backend('%'); putc_backend(*p);
            break;
        }
    }
    /* Flush the framebuffer once per printf block instead of character by character */
    if (s_fb_ready) {
        extern void compositor_render(void);
        compositor_render();
    }
}

/* ------------------------------------------------------------------ */
/* kprintf                                                             */
/* ------------------------------------------------------------------ */
void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vkprintf(fmt, ap);
    va_end(ap);
}

/* ------------------------------------------------------------------ */
/* kpanic_color / kpanic                                               */
/* ------------------------------------------------------------------ */
static void vkpanic_core(uint32_t bg, uint32_t fg, const char *fmt, va_list ap) {
    if (s_fb_ready) {
        fbc_set_bg(bg);
        fbc_set_fg(fg);
        fb_console_clear();
        fb_console_puts("\n\n  *** KERNEL PANIC ***\n  ");
    } else {
        vga_set_color(VGA_WHITE, VGA_RED);
        vga_puts("\n\n  *** KERNEL PANIC ***  \n  ");
    }

    vkprintf(fmt, ap);

    if (s_fb_ready) {
        fb_console_puts("\n\n  System halted.\n");
    } else {
        vga_puts("\n\n  System halted.\n");
    }
    panic_halt();
}

void kpanic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vkpanic_core(0x00AA0000, 0x00FFFFFF, fmt, ap);
    va_end(ap);
    panic_halt();
}

void kpanic_color(uint32_t bg, uint32_t fg, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vkpanic_core(bg, fg, fmt, ap);
    va_end(ap);
    panic_halt();
}
