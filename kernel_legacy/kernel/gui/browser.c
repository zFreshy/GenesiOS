/*
 * kernel/gui/browser.c
 * Native Genesi Web Browser
 */

#include "../include/kernel.h"
#include "../include/kprintf.h"
#include "window.h"
#include "../gfx/font.h"
#include "../net/ipv4.h"
#include "../net/tcp.h"
#include "../proc/scheduler.h"

extern volatile bool g_gui_needs_update;
extern int g_ui_scale;

static void browser_thread(void) {
    uint32_t w = 800 * g_ui_scale;
    uint32_t h = 600 * g_ui_scale;
    
    kprintf("  [Browser] Iniciando aplicativo nativo...\n");
    window_t *win = wm_create_window(200 * g_ui_scale, 100 * g_ui_scale, w, h, "Genesi Browser (Native)");
    if (!win || !win->buffer) {
        sched_exit();
        return;
    }

    /* Fundo branco */
    for (uint32_t i = 0; i < w * h; i++) {
        win->buffer[i] = 0xFFFFFFFF;
    }
    
    /* Barra de URL cinza */
    for (uint32_t y = 0; y < 40 * g_ui_scale; y++) {
        for (uint32_t x = 0; x < w; x++) {
            win->buffer[y * w + x] = 0xFFEEEEEE;
        }
    }

    font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 12 * g_ui_scale, "example.com", 0xFF000000, 0xFFEEEEEE, g_ui_scale);
    font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 50 * g_ui_scale, "Resolvendo dominio via DNS...", 0xFF0000FF, 0xFFFFFFFF, g_ui_scale);
    g_gui_needs_update = true;

    /* Resolver DNS usando a rotina do Kernel */
    extern void dns_resolve(const char *domain);
    extern uint8_t* dns_get_resolved_ip(void);

    dns_resolve("example.com");

    int wait_cycles = 1000;
    uint8_t *resolved_ip = NULL;
    while (!(resolved_ip = dns_get_resolved_ip()) && wait_cycles > 0) {
        extern void pit_sleep(uint32_t ms);
        pit_sleep(10); /* Libera a CPU para a interface redesenhar */
        wait_cycles--;
    }

    if (!resolved_ip) {
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 90 * g_ui_scale, "Erro: Falha na resolucao de DNS.", 0xFFFF0000, 0xFFFFFFFF, g_ui_scale);
        g_gui_needs_update = true;
        
        /* Fica vivo segurando a janela */
        for (;;) {
            extern void sched_yield(void);
            sched_yield();
        }
    }

    /* Limpa a area e escreve que conectou */
    for (uint32_t y = 50 * g_ui_scale; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) win->buffer[y * w + x] = 0xFFFFFFFF;
    }
    font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 50 * g_ui_scale, "DNS Resolvido! Conectando ao Servidor...", 0xFF00AA00, 0xFFFFFFFF, g_ui_scale);
    g_gui_needs_update = true;

    /* Conectar TCP */
    extern int tcp_connect(uint8_t *dst_ip, uint16_t dst_port);
    if (tcp_connect(resolved_ip, 80) < 0) {
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 90 * g_ui_scale, "Erro: TCP Connect falhou.", 0xFFFF0000, 0xFFFFFFFF, g_ui_scale);
        g_gui_needs_update = true;
        for (;;) {
            extern void sched_yield(void);
            sched_yield();
        }
    }

    font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 90 * g_ui_scale, "TCP Conectado! Baixando a pagina...", 0xFF00AA00, 0xFFFFFFFF, g_ui_scale);
    g_gui_needs_update = true;

    const char* req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
    extern void tcp_send_data(int sock, uint8_t *data, uint16_t len);
    tcp_send_data(0, (uint8_t *)req, kstrlen(req));

    /* Espera o payload (super simples, igual ao seu C++) */
    extern int tcp_get_rx_len(void);
    extern char* tcp_get_rx_buffer(void);
    
    wait_cycles = 1000;
    while (tcp_get_rx_len() == 0 && wait_cycles > 0) {
        extern void pit_sleep(uint32_t ms);
        pit_sleep(10);
        wait_cycles--;
    }

    /* Limpa a area */
    for (uint32_t y = 50 * g_ui_scale; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) win->buffer[y * w + x] = 0xFFFFFFFF;
    }

    if (tcp_get_rx_len() > 0) {
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 50 * g_ui_scale, "Pagina baixada com sucesso! Renderizando...", 0xFF000000, 0xFFFFFFFF, g_ui_scale);
        
        char *html = tcp_get_rx_buffer();
        int rx_len = tcp_get_rx_len();
        
        /* Mini parser HTML super basico (procura por titulo ou h1) */
        int text_y = 90 * g_ui_scale;
        int text_x = 10 * g_ui_scale;
        
        /* Renderiza o texto HTML na tela de forma bruta (limitado) */
        for (int i = 0; i < rx_len && text_y < (int)h - 20; i++) {
            if (html[i] == '\n') {
                text_y += FONT_HEIGHT * g_ui_scale + 5;
                text_x = 10 * g_ui_scale;
            } else if (html[i] >= 32 && html[i] <= 126) {
                if (text_x + FONT_WIDTH * g_ui_scale < (int)w) {
                    char c_str[2] = {html[i], '\0'};
                    font_draw_string_to_buffer_scaled(win->buffer, w, h, text_x, text_y, c_str, 0xFF222222, 0xFFFFFFFF, g_ui_scale);
                    text_x += FONT_WIDTH * g_ui_scale;
                } else {
                    text_y += FONT_HEIGHT * g_ui_scale + 5;
                    text_x = 10 * g_ui_scale;
                }
            }
        }
    } else {
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 10 * g_ui_scale, 50 * g_ui_scale, "Erro: Timeout no HTTP. Nenhum dado recebido.", 0xFFFF0000, 0xFFFFFFFF, g_ui_scale);
    }
    
    g_gui_needs_update = true;

    /* Loop eterno para manter a janela viva */
    for (;;) {
        extern void sched_yield(void);
        sched_yield();
    }
}

void launch_browser_app(void) {
    task_t *t = sched_create_task("browser", browser_thread);
    sched_add(t);
}
