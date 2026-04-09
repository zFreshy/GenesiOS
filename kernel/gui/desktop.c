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
extern int g_ui_scale;

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
    uint32_t w = 600 * g_ui_scale;
    uint32_t h = 400 * g_ui_scale;
    window_t *win = wm_create_window(100 * g_ui_scale, 100 * g_ui_scale, w, h, "Terminal");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < w * h; i++) {
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
    uint32_t w = 600 * g_ui_scale;
    uint32_t h = 420 * g_ui_scale;
    window_t *win = wm_create_window(250 * g_ui_scale, 150 * g_ui_scale, w, h, "File Explorer");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < w * h; i++) {
            win->buffer[i] = 0x00F0F5FA; /* Very light frosted blue/white */
        }
        /* Top bar for File Explorer */
        for (uint32_t y = 0; y < 56 * g_ui_scale; y++) {
            for (uint32_t x = 0; x < w; x++) {
                win->buffer[y * w + x] = 0x00FFFFFF; /* White header */
            }
        }
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 16 * g_ui_scale, 12 * g_ui_scale, "B:\\Modules", 0x004A5568, 0x00FFFFFF, g_ui_scale);
        
        int32_t draw_y = 76 * g_ui_scale;
        
        mb2_info_t *info = (mb2_info_t *)(uintptr_t)g_mboot_info;
        if (info) {
            mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)info + 8);
            while (tag->type != MB2_TAG_END) {
                if (tag->type == MB2_TAG_MODULE) {
                    mb2_module_tag_t *mod = (mb2_module_tag_t *)tag;
                    font_draw_string_to_buffer_scaled(win->buffer, w, h, 16 * g_ui_scale, draw_y, "\x09", 0x004A90E2, 0x00F0F5FA, g_ui_scale); /* Folder icon approx */
                    font_draw_string_to_buffer_scaled(win->buffer, w, h, 40 * g_ui_scale, draw_y, mod->string, 0x001A202C, 0x00F0F5FA, g_ui_scale);
                    
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
                    font_draw_string_to_buffer_scaled(win->buffer, w, h, 400 * g_ui_scale, draw_y, size_str, 0x00A0AEC0, 0x00F0F5FA, g_ui_scale);
                    
                    draw_y += 48 * g_ui_scale;
                }
                tag = (mb2_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
            }
        }
    }
}

void desktop_create_sysinfo(void) {
    uint32_t w = 440 * g_ui_scale;
    uint32_t h = 260 * g_ui_scale;
    window_t *win = wm_create_window(400 * g_ui_scale, 200 * g_ui_scale, w, h, "System Info");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < w * h; i++) {
            win->buffer[i] = 0x00202020; /* Dark Mica */
        }
        /* Accent header */
        for (uint32_t y = 0; y < 8 * g_ui_scale; y++) {
            for (uint32_t x = 0; x < w; x++) {
                win->buffer[y * w + x] = 0x000078D7; /* Windows Blue */
            }
        }
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 24 * g_ui_scale, "Genesi OS v11", 0x00FFFFFF, 0x00202020, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 80 * g_ui_scale, "Memory: 256 MB", 0x00AAAAAA, 0x00202020, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 120 * g_ui_scale, "CPU: x86 64-bit", 0x00AAAAAA, 0x00202020, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 160 * g_ui_scale, "GUI: W11 Dark Theme", 0x00AAAAAA, 0x00202020, g_ui_scale);
    }
}

void desktop_create_settings(void) {
    uint32_t w = 760 * g_ui_scale;
    uint32_t h = 500 * g_ui_scale;
    window_t *win = wm_create_window(150 * g_ui_scale, 150 * g_ui_scale, w, h, "Settings");
    if (win && win->buffer) {
        for (uint32_t i = 0; i < w * h; i++) {
            win->buffer[i] = 0x00F0F5FA; /* Light mode background */
        }
        /* Sidebar */
        for (uint32_t y = 0; y < h; y++) {
            for (uint32_t x = 0; x < 240 * g_ui_scale; x++) {
                win->buffer[y * w + x] = 0x00E8EDF2; /* Sidebar slightly darker */
            }
        }
        
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 30 * g_ui_scale, "Matheus Vinicius", 0x001A202C, 0x00E8EDF2, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 70 * g_ui_scale, "Local Account", 0x004A5568, 0x00E8EDF2, g_ui_scale);
        
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 140 * g_ui_scale, "System", 0x000078D7, 0x00E8EDF2, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 190 * g_ui_scale, "Personalization", 0x004A5568, 0x00E8EDF2, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 240 * g_ui_scale, "Network", 0x004A5568, 0x00E8EDF2, g_ui_scale);
        
        /* Main Area */
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 280 * g_ui_scale, 30 * g_ui_scale, "System Settings", 0x001A202C, 0x00F0F5FA, g_ui_scale);
        
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 280 * g_ui_scale, 100 * g_ui_scale, "Display", 0x001A202C, 0x00F0F5FA, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 280 * g_ui_scale, 140 * g_ui_scale, "Monitors, brightness", 0x00718096, 0x00F0F5FA, g_ui_scale);
        
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 280 * g_ui_scale, 210 * g_ui_scale, "Personalization", 0x001A202C, 0x00F0F5FA, g_ui_scale);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 280 * g_ui_scale, 250 * g_ui_scale, "Wallpaper", 0x00718096, 0x00F0F5FA, g_ui_scale);
        
        /* Wallpaper Buttons */
        /* Btn 1: Classic Gradient */
        for (uint32_t y = 300 * g_ui_scale; y < 350 * g_ui_scale; y++) {
            for (uint32_t x = 280 * g_ui_scale; x < 460 * g_ui_scale; x++) {
                win->buffer[y * w + x] = 0x00D0E8ED;
            }
        }
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 300 * g_ui_scale, 310 * g_ui_scale, "Gradient", 0x001A202C, 0x00D0E8ED, g_ui_scale);
        
        /* Btn 2: Image Wallpaper */
        for (uint32_t y = 300 * g_ui_scale; y < 350 * g_ui_scale; y++) {
            for (uint32_t x = 480 * g_ui_scale; x < 660 * g_ui_scale; x++) {
                win->buffer[y * w + x] = 0x00D0E8ED;
            }
        }
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 500 * g_ui_scale, 310 * g_ui_scale, "Picture", 0x001A202C, 0x00D0E8ED, g_ui_scale);
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
