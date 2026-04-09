#include <stdint.h>

/* Syscall wrapper */
extern "C" {
static inline uint64_t syscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    uint64_t ret;
    register uint64_t r10 __asm__("r10") = a4;
    register uint64_t r8  __asm__("r8")  = a5;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void write(int fd, const char *str, uint64_t count) {
    syscall(1, fd, (uint64_t)str, count, 0, 0);
}
void exit(int status) {
    syscall(0, status, 0, 0, 0, 0);
    while (1);
}
void* create_window(uint64_t width, uint64_t height, const char* title) {
    return (void*)syscall(10, width, height, (uint64_t)title, 0, 0);
}

// Network Ext
uint64_t dns_resolve(const char* domain, uint8_t* out_ip) {
    return syscall(20, (uint64_t)domain, (uint64_t)out_ip, 0, 0, 0);
}
uint64_t tcp_connect(uint8_t* ip, uint16_t port) {
    return syscall(21, (uint64_t)ip, port, 0, 0, 0);
}
uint64_t tcp_send(const char* data, uint16_t len) {
    return syscall(22, 0, (uint64_t)data, len, 0, 0); // sock=0 implicitly in kernel
}
uint64_t tcp_recv(char* buffer, uint16_t max_len) {
    return syscall(23, (uint64_t)buffer, max_len, 0, 0, 0);
}
void tcp_close() {
    syscall(24, 0, 0, 0, 0, 0);
}

// GUI Ext
struct sys_draw_rect_args {
    void *win; int32_t x, y, w, h; uint32_t color;
};
struct sys_draw_text_args {
    void *win; int32_t x, y; const char *str; uint32_t fg, bg;
};
void draw_rect(void* win, int x, int y, int w, int h, uint32_t color) {
    struct sys_draw_rect_args args = {win, x, y, w, h, color};
    syscall(30, (uint64_t)&args, 0, 0, 0, 0);
}
void draw_text(void* win, int x, int y, const char* str, uint32_t fg, uint32_t bg) {
    struct sys_draw_text_args args = {win, x, y, str, fg, bg};
    syscall(31, (uint64_t)&args, 0, 0, 0, 0);
}
int strlen(const char* s) { int i=0; while(s[i]) i++; return i; }
} // extern "C"

class Browser {
private:
    void* win;
    int win_w, win_h;

public:
    Browser(int w, int h) : win_w(w), win_h(h) {
        write(1, "[Browser] Iniciando GUI...\n", 27);
        win = create_window(w, h, "Genesi Web Browser");
        draw_rect(win, 0, 0, w, h, 0xFFFFFFFF); // White background
    }

    void navigate(const char* domain) {
        write(1, "[Browser] Resolvendo dominio...\n", 32);
        
        // Draw URL bar
        draw_rect(win, 0, 0, win_w, 40, 0xFFDDDDDD);
        draw_text(win, 10, 10, domain, 0xFF000000, 0xFFDDDDDD);

        uint8_t ip[4] = {0};
        if (!dns_resolve(domain, ip)) {
            write(1, "[Browser] DNS falhou.\n", 22);
            draw_text(win, 10, 50, "Erro: DNS Falhou (Cheque sua rede)", 0xFFFF0000, 0xFFFFFFFF);
            return;
        }

        draw_text(win, 10, 50, "Conectando ao servidor web...", 0xFF0000FF, 0xFFFFFFFF);
        if (!tcp_connect(ip, 80)) {
            draw_rect(win, 0, 41, win_w, win_h - 41, 0xFFFFFFFF);
            draw_text(win, 10, 50, "Erro: Falha na conexao TCP", 0xFFFF0000, 0xFFFFFFFF);
            return;
        }

        const char* req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        tcp_send(req, strlen(req));

        draw_text(win, 10, 90, "Request enviada. Baixando payload...", 0xFF0000FF, 0xFFFFFFFF);

        // Spin waiting for payload (naive, but enough for POC)
        char buffer[2048] = {0};
        int max_wait = 10000000; 
        int rx_len = 0;
        
        while (max_wait > 0) {
            rx_len = tcp_recv(buffer, 2047);
            if (rx_len > 0) break;
            max_wait--;
        }

        tcp_close();
        
        draw_rect(win, 0, 41, win_w, win_h - 41, 0xFFFFFFFF); // limpa a tela

        if (rx_len <= 0) {
            draw_text(win, 10, 50, "Erro: Timeout na resposta HTTP.", 0xFFFF0000, 0xFFFFFFFF);
            return;
        }

        write(1, "[Browser] Dados recebidos!\n", 27);
        render_html(buffer, rx_len);
    }

    void render_html(char* html, int len) {
        // Strip out HTTP headers
        char* body = html;
        for (int i=0; i<len-4; i++) {
            if (html[i]=='\r' && html[i+1]=='\n' && html[i+2]=='\r' && html[i+3]=='\n') {
                body = &html[i+4];
                break;
            }
        }

        // Basic HTML layout stripper
        char rendered[2048] = {0};
        int rend_i = 0;
        bool in_tag = false;
        
        for (int i=0; body[i] && i<len && rend_i < 2047; i++) {
            if (body[i] == '<') in_tag = true;
            else if (body[i] == '>') in_tag = false;
            else if (!in_tag) {
                // Ignore multiple spaces or new-lines to make it compact
                if (body[i] == '\n' || body[i] == '\r') continue;
                rendered[rend_i++] = body[i];
            }
        }
        rendered[rend_i] = '\0';
        
        // Print text with wrapping
        int y = 50;
        int x = 10;
        char line[120] = {0};
        int line_len = 0;
        
        for (int i=0; i < rend_i; i++) {
            if (line_len >= 55) { // Assuming 55 chars max width for 800px area
                line[line_len] = '\0';
                draw_text(win, x, y, line, 0xFF222222, 0xFFFFFFFF);
                y += 36; // Line height
                line_len = 0;
                if (y > win_h - 40) break; // Scroll stop
            }
            line[line_len++] = rendered[i];
        }
        if (line_len > 0) {
            line[line_len] = '\0';
            draw_text(win, x, y, line, 0xFF222222, 0xFFFFFFFF);
        }
    }
};

extern "C" void _start(void) {
    Browser my_browser(900, 700);
    my_browser.navigate("example.com");
    while (1) {} // Keep the window alive and task running
}
