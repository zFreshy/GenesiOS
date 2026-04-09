// user/browser/main.cpp
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
void memset(void* ptr, int val, int size) {
    char* p = (char*)ptr;
    while(size--) *p++ = val;
}
void memcpy(void* dst, const void* src, int size) {
    char* d = (char*)dst; const char* s = (const char*)src;
    while(size--) *d++ = *s++;
}
int strcmp(const char* s1, const char* s2) {
    while(*s1 && *s1 == *s2) { s1++; s2++; }
    return *s1 - *s2;
}
} // extern "C"

/* Simple Bump Allocator for User-Space (since we don't have malloc yet) */
static char g_heap[1024 * 1024]; // 1 MB heap
static int g_heap_idx = 0;

void* umalloc(int size) {
    if (g_heap_idx + size > sizeof(g_heap)) return 0;
    void* ptr = &g_heap[g_heap_idx];
    g_heap_idx += size;
    return ptr;
}
void ufree(void* ptr) { (void)ptr; /* No-op for now */ }
void heap_reset() { g_heap_idx = 0; }

/* Micro DOM Engine */
enum NodeType { NODE_TEXT, NODE_ELEMENT };

struct DOMNode {
    NodeType type;
    char tag[16];
    char* text; // If text node
    
    // Children
    DOMNode* children[64];
    int num_children;
    
    // Computed Layout Box
    int x, y, w, h;
    uint32_t color;
    uint32_t bg_color;
};

DOMNode* create_element(const char* tag) {
    DOMNode* n = (DOMNode*)umalloc(sizeof(DOMNode));
    memset(n, 0, sizeof(DOMNode));
    n->type = NODE_ELEMENT;
    int i=0; while(tag[i] && i<15) { n->tag[i] = tag[i]; i++; }
    n->tag[i] = 0;
    return n;
}

DOMNode* create_text_node(const char* txt, int len) {
    DOMNode* n = (DOMNode*)umalloc(sizeof(DOMNode));
    memset(n, 0, sizeof(DOMNode));
    n->type = NODE_TEXT;
    n->text = (char*)umalloc(len + 1);
    memcpy(n->text, txt, len);
    n->text[len] = 0;
    return n;
}

void append_child(DOMNode* parent, DOMNode* child) {
    if (parent->num_children < 64) {
        parent->children[parent->num_children++] = child;
    }
}

/* Very Naive HTML Parser */
DOMNode* parse_html(char* html, int len) {
    DOMNode* root = create_element("body");
    DOMNode* current = root;
    DOMNode* stack[32];
    int stack_ptr = 0;
    stack[stack_ptr++] = root;
    
    int i = 0;
    while (i < len) {
        if (html[i] == '<') {
            if (html[i+1] == '/') {
                // Close tag
                while(html[i] != '>' && i < len) i++;
                if (stack_ptr > 1) {
                    stack_ptr--;
                    current = stack[stack_ptr-1];
                }
            } else {
                // Open tag
                i++;
                char tag[16] = {0};
                int t = 0;
                while (html[i] != '>' && html[i] != ' ' && i < len && t < 15) {
                    tag[t++] = html[i++];
                }
                while(html[i] != '>' && i < len) i++; // skip attributes
                
                DOMNode* child = create_element(tag);
                append_child(current, child);
                
                // Self closing tags like <br> or <img/> shouldn't push to stack
                if (strcmp(tag, "br") != 0 && strcmp(tag, "img") != 0) {
                    if (stack_ptr < 32) {
                        stack[stack_ptr++] = child;
                        current = child;
                    }
                }
            }
            i++;
        } else {
            // Text content
            int start = i;
            while (html[i] != '<' && i < len) i++;
            int txt_len = i - start;
            
            // Trim leading/trailing whitespace a bit (naive)
            bool all_spaces = true;
            for(int j=0; j<txt_len; j++) if(html[start+j] != ' ' && html[start+j] != '\n' && html[start+j] != '\r') all_spaces = false;
            
            if (!all_spaces && txt_len > 0) {
                // Clean up newlines
                char* clean = (char*)umalloc(txt_len + 1);
                int c = 0;
                for(int j=0; j<txt_len; j++) {
                    if (html[start+j] != '\n' && html[start+j] != '\r') clean[c++] = html[start+j];
                }
                DOMNode* txt = create_text_node(clean, c);
                append_child(current, txt);
            }
        }
    }
    return root;
}

/* Layout Engine */
struct LayoutContext {
    int start_x, start_y;
    int max_w;
    int cur_x, cur_y;
};

void compute_layout(DOMNode* node, LayoutContext* ctx) {
    if (node->type == NODE_TEXT) {
        // Compute text box
        int len = strlen(node->text);
        int font_w = 8; // Naive 8x16 font assumed
        int font_h = 16;
        
        if (ctx->cur_x + len*font_w > ctx->start_x + ctx->max_w) {
            // Line break
            ctx->cur_x = ctx->start_x;
            ctx->cur_y += font_h + 4;
        }
        
        node->x = ctx->cur_x;
        node->y = ctx->cur_y;
        node->w = len * font_w;
        node->h = font_h;
        
        ctx->cur_x += node->w;
    } else {
        // Element Box
        if (strcmp(node->tag, "h1") == 0) {
            ctx->cur_x = ctx->start_x;
            ctx->cur_y += 24; // margin top
            node->color = 0xFF111111; // dark text
        } else if (strcmp(node->tag, "a") == 0) {
            node->color = 0xFF0000FF; // blue link
        } else if (strcmp(node->tag, "button") == 0) {
            node->bg_color = 0xFFEEEEEE;
            node->color = 0xFF000000;
            ctx->cur_x += 8; // padding
        } else if (strcmp(node->tag, "br") == 0) {
            ctx->cur_x = ctx->start_x;
            ctx->cur_y += 20;
        } else {
            node->color = 0xFF333333; // default text
        }
        
        int box_start_x = ctx->cur_x;
        int box_start_y = ctx->cur_y;
        
        // Layout children
        for (int i=0; i < node->num_children; i++) {
            // Inherit colors
            if (node->color) node->children[i]->color = node->color;
            if (node->bg_color) node->children[i]->bg_color = node->bg_color;
            
            compute_layout(node->children[i], ctx);
        }
        
        // Wrap box around children
        node->x = box_start_x;
        node->y = box_start_y;
        node->w = ctx->cur_x - box_start_x;
        node->h = ctx->cur_y - box_start_y + 16;
        
        // Block elements break line after
        if (strcmp(node->tag, "h1") == 0 || strcmp(node->tag, "p") == 0 || strcmp(node->tag, "div") == 0) {
            ctx->cur_x = ctx->start_x;
            ctx->cur_y += 24;
        } else if (strcmp(node->tag, "button") == 0) {
            ctx->cur_x += 16; // margin
        }
    }
}

/* Paint Engine */
void paint_dom(void* win, DOMNode* node) {
    if (node->type == NODE_TEXT) {
        uint32_t fg = node->color ? node->color : 0xFF222222;
        uint32_t bg = node->bg_color ? node->bg_color : 0xFFFFFFFF;
        draw_text(win, node->x, node->y, node->text, fg, bg);
    } else {
        if (node->bg_color) {
            draw_rect(win, node->x - 4, node->y - 4, node->w + 8, node->h + 8, node->bg_color);
        }
        if (strcmp(node->tag, "a") == 0) {
            // Draw underline for links
            draw_rect(win, node->x, node->y + 14, node->w, 2, 0xFF0000FF);
        }
        
        for (int i=0; i < node->num_children; i++) {
            paint_dom(win, node->children[i]);
        }
    }
}

class Browser {
private:
    void* win;
    int win_w, win_h;

public:
    Browser(int w, int h) : win_w(w), win_h(h) {
        write(1, "[Browser] Iniciando Micro-Engine...\n", 36);
        win = create_window(w, h, "Genesi Browser (HTML Engine)");
        draw_rect(win, 0, 0, w, h, 0xFFFFFFFF); // White background
    }

    void render_mock_html() {
        // This is what we would get from tcp_recv
        char mock_html[] = 
            "<html><body>"
            "<h1>Bem-vindo ao Genesi Web</h1>"
            "<p>Este e um <b>Micro Motor de Renderizacao HTML</b> rodando em user-space.</p>"
            "<br>"
            "<p>Ele constroi uma arvore DOM em memoria, processa os bounding boxes (Layout Engine) e pinta na tela (Paint Engine).</p>"
            "<br>"
            "<div><a href='http://genesi.os/'>Link para a Pagina Inicial</a></div>"
            "<br>"
            "<button>Clique Aqui (Mock)</button>"
            "</body></html>";
            
        render_html(mock_html, sizeof(mock_html)-1);
    }

    void render_html(char* html, int len) {
        heap_reset(); // Reset our bump allocator for the new page
        
        // Strip out HTTP headers if they exist
        char* body = html;
        for (int i=0; i<len-4; i++) {
            if (html[i]=='\r' && html[i+1]=='\n' && html[i+2]=='\r' && html[i+3]=='\n') {
                body = &html[i+4];
                len = len - (i + 4);
                break;
            }
        }

        // 1. Parsing Phase (Lexer -> DOM)
        DOMNode* dom = parse_html(body, len);
        
        // 2. Layout Phase
        LayoutContext ctx = {20, 60, win_w - 40, 20, 60}; // x, y, max_w, cur_x, cur_y
        compute_layout(dom, &ctx);
        
        // 3. Paint Phase
        draw_rect(win, 0, 41, win_w, win_h - 41, 0xFFFFFFFF); // limpa a tela
        paint_dom(win, dom);
    }
    
    void navigate(const char* domain) {
        write(1, "[Browser] Resolvendo dominio...\n", 32);
        
        // Draw URL bar
        draw_rect(win, 0, 0, win_w, 40, 0xFFDDDDDD);
        draw_text(win, 10, 10, domain, 0xFF000000, 0xFFDDDDDD);

        uint8_t ip[4] = {0};
        if (!dns_resolve(domain, ip)) {
            // Fallback to our mock HTML if no internet / no real DNS
            render_mock_html();
            return;
        }

        draw_text(win, 10, 50, "Conectando ao servidor web...", 0xFF0000FF, 0xFFFFFFFF);
        if (!tcp_connect(ip, 80)) {
            render_mock_html();
            return;
        }

        const char* req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        tcp_send(req, strlen(req));

        draw_text(win, 10, 90, "Request enviada. Baixando payload...", 0xFF0000FF, 0xFFFFFFFF);

        char buffer[4096] = {0}; // Increased buffer
        int max_wait = 10000000; 
        int rx_len = 0;
        
        while (max_wait > 0) {
            rx_len = tcp_recv(buffer, 4095);
            if (rx_len > 0) break;
            max_wait--;
        }

        tcp_close();
        
        if (rx_len <= 0) {
            render_mock_html();
            return;
        }

        write(1, "[Browser] Dados recebidos!\n", 27);
        render_html(buffer, rx_len);
    }
};

extern "C" void _start(void) {
    Browser my_browser(900, 700);
    my_browser.navigate("genesi.os"); // Tries network, falls back to local Micro Engine Demo
    while (1) {} // Keep the window alive and task running
}
