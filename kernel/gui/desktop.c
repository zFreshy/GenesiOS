/*
 * kernel/gui/desktop.c
 * Basic OS desktop layout and window creation.
 */
#include "desktop.h"
#include "window.h"
#include "compositor.h"
#include "../gfx/framebuffer.h"
#include "../gfx/fb_console.h"
#include "../mm/heap.h"
#include "../gfx/font.h"

extern void shell_exec(const char *cmd);

static char s_term_line[256];
static size_t s_term_len = 0;

static void terminal_on_key(window_t *win, char c) {
    fb_console_bind_window(win);
    
    if (c == '\n' || c == '\r') {
        fbc_putchar('\n');
        s_term_line[s_term_len] = '\0';
        shell_exec(s_term_line);
        s_term_len = 0;
        
        fbc_set_fg(0x0055FF55);
        fbc_puts("genesi");
        fbc_set_fg(0x00FFFFFF);
        fbc_puts("> ");
    } else if (c == '\b') {
        if (s_term_len > 0) {
            s_term_len--;
            fbc_putchar('\b');
            fbc_putchar(' ');
            fbc_putchar('\b');
        }
    } else {
        if (s_term_len < sizeof(s_term_line) - 1) {
            s_term_line[s_term_len++] = c;
            fbc_putchar(c);
        }
    }
    
    /* Atualiza a janela para refletir a digitação atual */
    compositor_render();
}

void desktop_create_terminal(void) {
    window_t *win = wm_create_window(100, 100, 600, 400, "Terminal");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 600 * 400; i++) {
            win->buffer[i] = 0x00181818; /* Dark grey */
        }
        win->on_key = terminal_on_key;
        
        fb_console_bind_window(win);
        fbc_set_bg(0x00181818);
        fbc_clear(); /* Limpa o buffer com a cor certa para garantir um console limpo */
        
        fbc_set_fg(0x00FFFFFF);
        fbc_puts("Genesi OS Terminal v0.2\n\n");
        
        fbc_set_fg(0x0055FF55);
        fbc_puts("genesi");
        fbc_set_fg(0x00FFFFFF);
        fbc_puts("> ");
    }
}

#include "../include/multiboot2.h"

extern uint64_t g_mboot_info;

void desktop_create_explorer(void) {
    window_t *win = wm_create_window(250, 150, 500, 350, "File Explorer");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 500 * 350; i++) {
            win->buffer[i] = 0x00202020; /* Dark Mica */
        }
        /* Top bar for File Explorer */
        for (uint32_t y = 0; y < 40; y++) {
            for (uint32_t x = 0; x < 500; x++) {
                win->buffer[y * 500 + x] = 0x001A1A1A; /* Darker header */
            }
        }
        font_draw_string_to_buffer(win->buffer, 500, 350, 16, 10, "B:\\Modules", 0x00E0E0E0, 0x001A1A1A);
        
        int32_t draw_y = 50;
        
        mb2_info_t *info = (mb2_info_t *)(uintptr_t)g_mboot_info;
        if (info) {
            mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)info + 8);
            while (tag->type != MB2_TAG_END) {
                if (tag->type == MB2_TAG_MODULE) {
                    mb2_module_tag_t *mod = (mb2_module_tag_t *)tag;
                    font_draw_string_to_buffer(win->buffer, 500, 350, 16, draw_y, "\x09", 0x000078D7, 0x00202020); /* Folder icon approx */
                    font_draw_string_to_buffer(win->buffer, 500, 350, 40, draw_y, mod->string, 0x00E0E0E0, 0x00202020);
                    
                    /* Print size */
                    uint32_t size = mod->mod_end - mod->mod_start;
                    char size_str[32];
                    /* Quick int to string */
                    uint32_t temp = size;
                    int i = 0;
                    if (temp == 0) { size_str[i++] = '0'; }
                    while (temp > 0) { size_str[i++] = (char)('0' + (temp % 10)); temp /= 10; }
                    size_str[i++] = ' '; size_str[i++] = 'B'; size_str[i++] = 'y'; size_str[i++] = 't'; size_str[i++] = 'e'; size_str[i++] = 's'; size_str[i] = '\0';
                    /* Reverse string */
                    for (int j = 0; j < (i-7)/2; j++) {
                        char t = size_str[j];
                        size_str[j] = size_str[i - 8 - j];
                        size_str[i - 8 - j] = t;
                    }
                    font_draw_string_to_buffer(win->buffer, 500, 350, 350, draw_y, size_str, 0x00AAAAAA, 0x00202020);
                    
                    draw_y += 24;
                }
                tag = (mb2_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
            }
        }
    }
}

void desktop_create_sysinfo(void) {
    window_t *win = wm_create_window(400, 200, 300, 200, "System Info");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < 300 * 200; i++) {
            win->buffer[i] = 0x00202020; /* Dark Mica */
        }
        /* Accent header */
        for (uint32_t y = 0; y < 8; y++) {
            for (uint32_t x = 0; x < 300; x++) {
                win->buffer[y * 300 + x] = 0x000078D7; /* Windows Blue */
            }
        }
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 24, "Genesi OS v11", 0x00FFFFFF, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 48, "Memory: 256 MB", 0x00AAAAAA, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 64, "CPU: x86 64-bit", 0x00AAAAAA, 0x00202020);
        font_draw_string_to_buffer(win->buffer, 300, 200, 16, 80, "GUI: W11 Dark Theme", 0x00AAAAAA, 0x00202020);
    }
}

/* ------------------------------------------------------------------ */
/* desktop_start                                                      */
/* ------------------------------------------------------------------ */
void desktop_start(void) {
    if (!fb_available()) return;

    wm_init();
    compositor_init();

    /* Create a couple of demo windows for the compositor to draw */
    desktop_create_explorer();
    desktop_create_terminal(); /* Terminal criado por último recebe o FOCO automaticamente! */

    /* We render once immediately */
    compositor_update();
}
