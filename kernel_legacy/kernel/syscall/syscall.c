/*
 * kernel/syscall/syscall.c
 * System call dispatch table and basic handlers.
 */
#include "syscall.h"
#include "../include/kprintf.h"
#include "../proc/scheduler.h"
#include "../gui/window.h"

/* The assembly entry point */
extern void syscall_entry(void);

/* Helpers to read/write MSRs */
static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = (uint32_t)val;
    uint32_t high = (uint32_t)(val >> 32);
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high) : "memory");
}

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
    return ((uint64_t)high << 32) | low;
}

/* ------------------------------------------------------------------ */
/* Handlers                                                             */
/* ------------------------------------------------------------------ */
static uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t count) {
    if (fd != 1 && fd != 2) return (uint64_t)-1; // Only stdout/stderr for now
    const char *str = (const char *)(uintptr_t)buf;
    for (uint64_t i = 0; i < count; i++) {
        kprintf("%c", str[i]);
    }
    return count;
}

static uint64_t sys_getpid(void) {
    task_t *t = sched_current();
    if (t) return t->pid;
    return 0;
}

static uint64_t sys_exit(uint64_t status) {
    kprintf("\n[SYSCALL] Process exited with status %llu\n", status);
    sched_exit();
    return 0; // Never reached
}

/* GUI Syscalls */
static uint64_t sys_create_window(uint64_t width, uint64_t height, uint64_t title_ptr) {
    window_t *win = wm_create_window(100, 100, width, height, (const char *)(uintptr_t)title_ptr);
    
    extern volatile bool g_gui_needs_update;
    g_gui_needs_update = true;
    
    return (uint64_t)win;
}

/* Networking & GUI Ext */
extern int tcp_connect(uint8_t *dst_ip, uint16_t dst_port);
extern void tcp_send_data(int sock, uint8_t *data, uint16_t len);
extern void tcp_close(int sock);
extern int tcp_get_rx_len(void);
extern char* tcp_get_rx_buffer(void);
extern void dns_resolve(const char *domain);
extern uint8_t* dns_get_resolved_ip(void);
extern void pit_sleep(uint32_t ms);

static uint64_t sys_dns_resolve(uint64_t domain_ptr, uint64_t out_ip_ptr) {
    dns_resolve((const char*)(uintptr_t)domain_ptr);
    int timeout = 50; 
    uint8_t *ip = NULL;
    while(timeout > 0) {
        ip = dns_get_resolved_ip();
        if (ip) break;
        pit_sleep(100);
        timeout--;
        if (timeout % 10 == 0) dns_resolve((const char*)(uintptr_t)domain_ptr);
    }
    if(ip) {
        uint8_t *out = (uint8_t*)(uintptr_t)out_ip_ptr;
        out[0] = ip[0]; out[1] = ip[1]; out[2] = ip[2]; out[3] = ip[3];
        return 1;
    }
    return 0;
}

static uint64_t sys_tcp_connect(uint64_t ip_ptr, uint64_t port) {
    return tcp_connect((uint8_t*)(uintptr_t)ip_ptr, port);
}

static uint64_t sys_tcp_send(uint64_t sock, uint64_t data_ptr, uint64_t len) {
    tcp_send_data((int)sock, (uint8_t*)(uintptr_t)data_ptr, (uint16_t)len);
    return len;
}

static uint64_t sys_tcp_recv(uint64_t buf_ptr, uint64_t max_len) {
    int rx_len = tcp_get_rx_len();
    if (rx_len <= 0) return 0;
    char *rx_buf = tcp_get_rx_buffer();
    char *user_buf = (char*)(uintptr_t)buf_ptr;
    
    /* VERY simple kmemcpy */
    extern void* kmemcpy(void *dest, const void *src, size_t n);
    uint32_t to_copy = (rx_len > (int)max_len) ? (int)max_len : rx_len;
    kmemcpy(user_buf, rx_buf, to_copy);
    return to_copy;
}

static uint64_t sys_tcp_close(uint64_t sock) {
    tcp_close((int)sock);
    return 0;
}

/* GUI Ext */
extern void font_draw_string_to_buffer_scaled(uint32_t *buffer, uint32_t w, uint32_t h, uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg, uint32_t scale);
extern int g_ui_scale;

struct sys_draw_rect_args {
    void *win;
    int32_t x, y, w, h;
    uint32_t color;
};

struct sys_draw_text_args {
    void *win;
    int32_t x, y;
    const char *str;
    uint32_t fg, bg;
};

static uint64_t sys_draw_rect(uint64_t arg_ptr) {
    struct sys_draw_rect_args *a = (struct sys_draw_rect_args*)(uintptr_t)arg_ptr;
    window_t *win = (window_t*)a->win;
    if(!win || !win->buffer) return 0;
    
    for(int32_t cy = a->y; cy < a->y + a->h; cy++) {
        if(cy < 0 || cy >= (int32_t)win->height) continue;
        for(int32_t cx = a->x; cx < a->x + a->w; cx++) {
            if(cx < 0 || cx >= (int32_t)win->width) continue;
            win->buffer[cy * win->width + cx] = a->color;
        }
    }
    extern volatile bool g_gui_needs_update;
    g_gui_needs_update = true;
    return 1;
}

static uint64_t sys_draw_text(uint64_t arg_ptr) {
    struct sys_draw_text_args *a = (struct sys_draw_text_args*)(uintptr_t)arg_ptr;
    window_t *win = (window_t*)a->win;
    if(!win || !win->buffer) return 0;
    font_draw_string_to_buffer_scaled(win->buffer, win->width, win->height, a->x, a->y, a->str, a->fg, a->bg, g_ui_scale);
    extern volatile bool g_gui_needs_update;
    g_gui_needs_update = true;
    return 1;
}

/* ------------------------------------------------------------------ */
/* syscall_init                                                         */
/* ------------------------------------------------------------------ */
void syscall_init(void) {
    /* Enable SCE (Syscall Enable) in EFER */
    uint64_t efer = rdmsr(MSR_EFER);
    wrmsr(MSR_EFER, efer | 1);

    /* 
     * Set up STAR (Segment Selector Register)
     * Bits 32-47: Kernel CS/SS base (CS = STAR[47:32], SS = STAR[47:32] + 8) -> CS=0x08, SS=0x10
     * Bits 48-63: User CS/SS base (CS = STAR[63:48] + 16, SS = STAR[63:48] + 8) -> CS=0x20, SS=0x18
     * Thus, we set STAR[47:32] = 0x08, STAR[63:48] = 0x10.
     */
    uint64_t star = ((0x10ULL) << 48) | ((0x08ULL) << 32);
    wrmsr(MSR_STAR, star);

    /* Set up LSTAR (Long Mode Syscall Target Address Register) */
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);

    /* Set up SFMASK (Syscall Flag Mask) - disable interrupts during syscall */
    wrmsr(MSR_SFMASK, 0x200); /* Disable interrupts (IF bit) */
}

/* ------------------------------------------------------------------ */
/* syscall_handler (called from ASM)                                   */
/* ------------------------------------------------------------------ */
uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    (void)a4; (void)a5;
    
    kprintf("[SYSCALL] num=%llu called by user!\n", num);

    switch (num) {
        case SYS_EXIT:   return sys_exit(a1);
        case SYS_WRITE:  return sys_write(a1, a2, a3);
        case SYS_GETPID: return sys_getpid();
        
        /* GUI/Network extensions */
        case 10: return sys_create_window(a1, a2, a3);
        case 20: return sys_dns_resolve(a1, a2);
        case 21: return sys_tcp_connect(a1, a2);
        case 22: return sys_tcp_send(a1, a2, a3);
        case 23: return sys_tcp_recv(a1, a2);
        case 24: return sys_tcp_close(a1);
        
        case 30: return sys_draw_rect(a1);
        case 31: return sys_draw_text(a1);
        
        default:
            kprintf("[SYSCALL] Unknown syscall: %llu\n", num);
            return (uint64_t)-1;
    }
}
