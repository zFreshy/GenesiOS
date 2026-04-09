/*
 * kernel/drivers/keyboard.c
 * PS/2 keyboard driver — scancode set 1, US QWERTY layout.
 * Drives via IRQ1, stores chars in a small ring buffer.
 */
#include "keyboard.h"
#include "../arch/x86_64/irq.h"
#include "../include/vga.h"
#include "../include/kernel.h"

#define KB_DATA   0x60
#define KB_STATUS 0x64

/* ------------------------------------------------------------------ */
/* US QWERTY scancode set 1 -> ASCII  (index = scancode)              */
/* ------------------------------------------------------------------ */
static const char s_sc_normal[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8','9','-','4','5','6','+','1','2','3','0','.', 0, 0, 0,
};
static const char s_sc_shift[128] = {
    0,   27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"', '~',
    0,   '|', 'Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0,  ' ',
};

/* ------------------------------------------------------------------ */
/* Ring buffer                                                          */
/* ------------------------------------------------------------------ */
static char     s_buf[KB_BUF_SIZE];
static uint32_t s_head = 0, s_tail = 0;
static bool     s_shift = false;
static bool     s_caps  = false;
static bool     s_e0    = false;

static void buf_push(char c) {
    uint32_t next = (s_head + 1) % KB_BUF_SIZE;
    if (next != s_tail) { s_buf[s_head] = c; s_head = next; }
}

static char buf_pop(void) {
    if (s_head == s_tail) return 0;
    char c = s_buf[s_tail];
    s_tail = (s_tail + 1) % KB_BUF_SIZE;
    return c;
}

/* ------------------------------------------------------------------ */
/* IRQ1 handler                                                         */
/* ------------------------------------------------------------------ */
static void kb_irq_handler(registers_t *regs) {
    (void)regs;

    uint8_t sc = inb(KB_DATA);

    if (sc == 0xE0) {
        s_e0 = true;
        irq_send_eoi(IRQ_KEYBOARD);
        return;
    }

    bool is_e0 = s_e0;
    s_e0 = false;

    /* Key release — high bit set */
    if (sc & 0x80) {
        sc &= 0x7F;
        if (sc == 0x2A || sc == 0x36) s_shift = false;
        irq_send_eoi(IRQ_KEYBOARD);
        return;
    }

    /* Handle F11 Fullscreen toggle */
    if (sc == 0x57) {
        extern void compositor_toggle_fullscreen(void);
        compositor_toggle_fullscreen();
        irq_send_eoi(IRQ_KEYBOARD);
        return;
    }

    /* Handle Windows Key (Left GUI = 0x5B, Right GUI = 0x5C) or F12 (0x58) */
    if ((is_e0 && (sc == 0x5B || sc == 0x5C)) || sc == 0x58) {
        extern void compositor_toggle_start_menu(void);
        compositor_toggle_start_menu();
        irq_send_eoi(IRQ_KEYBOARD);
        return;
    }

    /* Modifiers */
    if (sc == 0x2A || sc == 0x36) { s_shift = true;  irq_send_eoi(IRQ_KEYBOARD); return; }
    if (sc == 0x3A) { s_caps = !s_caps;               irq_send_eoi(IRQ_KEYBOARD); return; }

    /* Map to ASCII */
    char c = 0;
    if (sc < 128) {
        if (s_shift)
            c = (sc < sizeof(s_sc_shift)) ? s_sc_shift[sc] : 0;
        else
            c = s_sc_normal[sc];

        /* Apply caps-lock to letters */
        if (s_caps && c >= 'a' && c <= 'z') c = (char)(c - 32);
        if (s_caps && c >= 'A' && c <= 'Z') c = (char)(c + 32);
    }

    if (c) buf_push(c);

    irq_send_eoi(IRQ_KEYBOARD);
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */
void keyboard_init(void) {
    s_head = s_tail = 0;
    irq_register_handler(IRQ_KEYBOARD, kb_irq_handler);
    irq_unmask(IRQ_KEYBOARD);
}

bool keyboard_has_char(void) {
    return s_head != s_tail;
}

char keyboard_getchar(void) {
    while (!keyboard_has_char()) {
        extern void sched_yield(void);
        sched_yield();
    }
    return buf_pop();
}
