/*
 * kernel/kprintf.c
 * Minimal kernel-space printf: outputs to VGA via vga_putchar().
 * Supported: %c %s %d %i %u %x %X %p %zu %llu %%
 */
#include "include/kprintf.h"
#include "include/vga.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */
static void print_uint(uint64_t v, int base, bool upper) {
    static const char lo[] = "0123456789abcdef";
    static const char hi[] = "0123456789ABCDEF";
    const char *digits = upper ? hi : lo;
    char buf[65]; int len = 0;
    if (v == 0) { vga_putchar('0'); return; }
    while (v) { buf[len++] = digits[v % (uint64_t)base]; v /= (uint64_t)base; }
    for (int i = len - 1; i >= 0; i--) vga_putchar(buf[i]);
}

static void print_int(int64_t v) {
    if (v < 0) { vga_putchar('-'); v = -v; }
    print_uint((uint64_t)v, 10, false);
}

/* ------------------------------------------------------------------ */
/* vkprintf                                                            */
/* ------------------------------------------------------------------ */
void vkprintf(const char *fmt, va_list ap) {
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { vga_putchar(*p); continue; }
        p++;
        switch (*p) {
        case 'c':
            vga_putchar((char)va_arg(ap, int));
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            vga_puts(s ? s : "(null)");
            break;
        }
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
            vga_puts("0x");
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
            } else if (*(p+1) == 'u') { p++;
                print_uint((uint64_t)va_arg(ap, unsigned long), 10, false);
            }
            break;
        case '%':
            vga_putchar('%');
            break;
        default:
            vga_putchar('%'); vga_putchar(*p);
            break;
        }
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
/* kpanic                                                              */
/* ------------------------------------------------------------------ */
void kpanic(const char *fmt, ...) {
    vga_set_color(VGA_WHITE, VGA_RED);
    vga_puts("\n\n  *** KERNEL PANIC ***  \n  ");

    va_list ap;
    va_start(ap, fmt);
    vkprintf(fmt, ap);
    va_end(ap);

    vga_puts("\n\n  System halted.\n");
    panic_halt();
}
