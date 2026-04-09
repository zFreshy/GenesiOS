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
}

static void terminal_on_resize(window_t *win) {
    if (!win || !win->buffer) return;
    uint32_t w = win->width;
    uint32_t h = win->height;
    
    /* We don't want to clear the whole terminal on resize, but wm_resize_window 
       allocates a new buffer with 0s. We should ideally only fill the new 0s 
       with the background color. */
    for (uint32_t i = 0; i < w * h; i++) {
        if (win->buffer[i] == 0) {
            win->buffer[i] = 0x00181A1F; /* Dark blueish grey */
        }
    }
}

void desktop_create_terminal(void) {
    uint32_t w = 680 * g_ui_scale;
    uint32_t h = 500 * g_ui_scale;
    window_t *win = wm_create_window(350 * g_ui_scale, 200 * g_ui_scale, w, h, "Terminal");
    if (win && win->buffer) {
        win->on_key = terminal_on_key;
        win->on_resize = terminal_on_resize;
        
        for (uint32_t i = 0; i < w * h; i++) {
            win->buffer[i] = 0x00181A1F; /* Dark blueish grey */
        }
        
        fb_console_bind_window(win);
        fbc_set_bg(0x00181A1F);
        fbc_clear(); 
        
        fbc_set_fg(0x00FFFFFF);
        fbc_puts("Genesi OS Terminal v0.3\n\n");
        
        fbc_set_fg(0x0055FF55);
        fbc_puts("genesi");
        fbc_set_fg(0x00FFFFFF);
        fbc_puts("> ");
    }
}

#include "../include/multiboot2.h"

extern uint64_t g_mboot_info;

#include "icons/icon_folder.h"
#include "icons/icon_doc.h"
#include "icons/icon_image.h"
#include "icons/icon_video.h"
#include "icons/icon_add.h"
#include "icons/icon_search.h"

static void draw_icon_to_buffer(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, const uint32_t *icon_data, uint32_t icon_w, uint32_t icon_h) {
    if (!buffer) return;
    int32_t scaled_w = icon_w * g_ui_scale;
    int32_t scaled_h = icon_h * g_ui_scale;
    
    for (int32_t iy = 0; iy < scaled_h; iy++) {
        for (int32_t ix = 0; ix < scaled_w; ix++) {
            if (x + ix < 0 || x + ix >= (int32_t)buf_w || y + iy < 0 || y + iy >= (int32_t)buf_h) continue;
            
            uint32_t src_x = (ix << 8) / g_ui_scale;
            uint32_t src_y = (iy << 8) / g_ui_scale;
            
            uint32_t x0 = src_x >> 8;
            uint32_t y0 = src_y >> 8;
            uint32_t x1 = x0 + 1;
            uint32_t y1 = y0 + 1;
            if (x1 >= icon_w) x1 = icon_w - 1;
            if (y1 >= icon_h) y1 = icon_h - 1;
            
            uint32_t fx = src_x & 0xFF;
            uint32_t fy = src_y & 0xFF;
            
            uint32_t c00 = icon_data[y0 * icon_w + x0];
            uint32_t c01 = icon_data[y0 * icon_w + x1];
            uint32_t c10 = icon_data[y1 * icon_w + x0];
            uint32_t c11 = icon_data[y1 * icon_w + x1];
            
            uint32_t a00 = (c00 >> 24) & 0xFF, a01 = (c01 >> 24) & 0xFF;
            uint32_t a10 = (c10 >> 24) & 0xFF, a11 = (c11 >> 24) & 0xFF;
            uint32_t top_a = (a00 * (256 - fx) + a01 * fx) >> 8;
            uint32_t bot_a = (a10 * (256 - fx) + a11 * fx) >> 8;
            uint32_t alpha = (top_a * (256 - fy) + bot_a * fy) >> 8;
            
            if (alpha > 0) {
                /* For white tinted SVG icons */
                uint32_t fr = 255, fg = 255, fb = 255;
                
                if (alpha == 255) {
                    buffer[(y + iy) * buf_w + (x + ix)] = (fr << 16) | (fg << 8) | fb;
                } else {
                    uint32_t bg = buffer[(y + iy) * buf_w + (x + ix)];
                    uint32_t bgr = (bg >> 16) & 0xFF;
                    uint32_t bgg = (bg >> 8) & 0xFF;
                    uint32_t bgb = bg & 0xFF;
                    
                    uint32_t r = (fr * alpha + bgr * (255 - alpha)) / 255;
                    uint32_t g = (fg * alpha + bgg * (255 - alpha)) / 255;
                    uint32_t b = (fb * alpha + bgb * (255 - alpha)) / 255;
                    
                    buffer[(y + iy) * buf_w + (x + ix)] = (r << 16) | (g << 8) | b;
                }
            }
        }
    }
}

static void draw_rounded_rect_buffer(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color) {
    for (int32_t cy = y; cy < y + h; cy++) {
        for (int32_t cx = x; cx < x + w; cx++) {
            if (cx < 0 || cx >= (int32_t)buf_w || cy < 0 || cy >= (int32_t)buf_h) continue;
            
            int32_t dx = 0, dy = 0;
            if (cx < x + r) dx = (x + r - 1) - cx;
            else if (cx >= x + w - r) dx = cx - (x + w - r);
            if (cy < y + r) dy = (y + r - 1) - cy;
            else if (cy >= y + h - r) dy = cy - (y + h - r);
            
            if (dx*dx + dy*dy >= r*r) continue;
            buffer[cy * buf_w + cx] = color;
        }
    }
}

#include "../fs/vfs.h"

struct explorer_data {
    char current_path[256];
    int scroll_y;
    int sidebar_width;
    int selected_sidebar;    /* -1=none, 0=home, 1=documents, 2=pictures, 10=disk C */
    bool qa_open;            /* Quick Access section expanded */
    bool pc_open;            /* This PC section expanded */
};

/* Helper: draw a horizontal line (separator) in buffer */
static void draw_hline_buffer(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, int32_t w, uint32_t color) {
    if (y < 0 || y >= (int32_t)buf_h) return;
    for (int32_t i = 0; i < w; i++) {
        int32_t px = x + i;
        if (px >= 0 && px < (int32_t)buf_w)
            buffer[y * buf_w + px] = color;
    }
}

/* Helper: fill a rect in buffer */
static void fill_rect_buffer(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    for (int32_t cy = y; cy < y + h; cy++) {
        if (cy < 0 || cy >= (int32_t)buf_h) continue;
        for (int32_t cx = x; cx < x + w; cx++) {
            if (cx < 0 || cx >= (int32_t)buf_w) continue;
            buffer[cy * buf_w + cx] = color;
        }
    }
}

/* Helper: draw a small 16x16 folder glyph into buffer using rectangles */
static void draw_mini_folder(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, uint32_t color) {
    int s = g_ui_scale;
    /* Tab part */
    fill_rect_buffer(buffer, buf_w, buf_h, x, y + 2*s, 7*s, 3*s, color);
    /* Body */
    fill_rect_buffer(buffer, buf_w, buf_h, x, y + 5*s, 14*s, 9*s, color);
}

/* Helper: draw a small disk icon using rectangles */
static void draw_mini_disk(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, uint32_t color) {
    int s = g_ui_scale;
    fill_rect_buffer(buffer, buf_w, buf_h, x + 1*s, y + 2*s, 12*s, 10*s, color);
    /* Top highlight */
    fill_rect_buffer(buffer, buf_w, buf_h, x + 2*s, y + 3*s, 10*s, 2*s, 0x00556677);
}

/* Helper: draw a chevron (▾ or ▸) */
static void draw_chevron_down(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, uint32_t color) {
    int s = g_ui_scale;
    /* Simple down arrow: 3 lines */
    fill_rect_buffer(buffer, buf_w, buf_h, x, y, 7*s, 1*s, color);
    fill_rect_buffer(buffer, buf_w, buf_h, x + 1*s, y + 1*s, 5*s, 1*s, color);
    fill_rect_buffer(buffer, buf_w, buf_h, x + 2*s, y + 2*s, 3*s, 1*s, color);
    fill_rect_buffer(buffer, buf_w, buf_h, x + 3*s, y + 3*s, 1*s, 1*s, color);
}

static void draw_chevron_right(uint32_t *buffer, uint32_t buf_w, uint32_t buf_h, int32_t x, int32_t y, uint32_t color) {
    int s = g_ui_scale;
    fill_rect_buffer(buffer, buf_w, buf_h, x, y, 1*s, 7*s, color);
    fill_rect_buffer(buffer, buf_w, buf_h, x + 1*s, y + 1*s, 1*s, 5*s, color);
    fill_rect_buffer(buffer, buf_w, buf_h, x + 2*s, y + 2*s, 1*s, 3*s, color);
    fill_rect_buffer(buffer, buf_w, buf_h, x + 3*s, y + 3*s, 1*s, 1*s, color);
}

static void explorer_draw_path(window_t *win) {
    if (!win || !win->buffer) return;
    struct explorer_data *data = (struct explorer_data *)win->user_data;
    uint32_t w = win->width;
    uint32_t h = win->height;
    int s = g_ui_scale;
    
    int32_t sidebar_w = data->sidebar_width;
    int32_t navbar_h = 48 * s;
    int32_t statusbar_h = 28 * s;
    
    /* ============================================================ */
    /* 1. Fill entire background                                     */
    /* ============================================================ */
    uint32_t bg_content = 0x001C1E23;
    uint32_t bg_sidebar = 0x00252729;
    uint32_t bg_navbar  = 0x002B2D31;
    uint32_t bg_status  = 0x00252729;
    uint32_t col_separator = 0x003A3C40;
    uint32_t col_highlight = 0x003B4455;
    uint32_t col_text_white = 0x00FFFFFF;
    uint32_t col_text_grey  = 0x00A0A0A0;
    uint32_t col_text_dim   = 0x00707070;
    uint32_t col_accent     = 0x000078D7;
    
    /* Content area */
    for (uint32_t i = 0; i < w * h; i++) {
        win->buffer[i] = bg_content;
    }
    
    /* Sidebar background */
    fill_rect_buffer(win->buffer, w, h, 0, navbar_h, sidebar_w, h - navbar_h, bg_sidebar);
    
    /* Navbar background */
    fill_rect_buffer(win->buffer, w, h, 0, 0, w, navbar_h, bg_navbar);
    
    /* Statusbar background */
    fill_rect_buffer(win->buffer, w, h, 0, h - statusbar_h, w, statusbar_h, bg_status);
    
    /* Separator lines */
    draw_hline_buffer(win->buffer, w, h, 0, navbar_h - 1, w, col_separator);
    draw_hline_buffer(win->buffer, w, h, 0, h - statusbar_h, w, col_separator);
    /* Vertical separator between sidebar and content */
    for (int32_t y = navbar_h; y < (int32_t)(h - statusbar_h); y++) {
        if (sidebar_w > 0 && sidebar_w < (int32_t)w)
            win->buffer[y * w + sidebar_w - 1] = col_separator;
    }
    
    /* ============================================================ */
    /* 2. Navbar: Back arrow + Breadcrumb path + Search icon         */
    /* ============================================================ */
    int32_t nav_text_y = (navbar_h - 16 * s) / 2;
    int32_t nav_x = 12 * s;
    
    /* Back button (always visible, dimmed if at root) */
    {
        uint32_t back_color = (kstrcmp(data->current_path, "/") != 0) ? col_text_white : col_text_dim;
        font_draw_string_to_buffer_scaled(win->buffer, w, h, nav_x, nav_text_y, "<", back_color, bg_navbar, s);
        nav_x += 16 * s;
    }
    
    /* Separator */
    fill_rect_buffer(win->buffer, w, h, nav_x, 10 * s, 1 * s, navbar_h - 20 * s, col_separator);
    nav_x += 8 * s;
    
    /* Folder icon in navbar */
    draw_mini_folder(win->buffer, w, h, nav_x, nav_text_y, 0x00F5A623);
    nav_x += 18 * s;
    
    /* Breadcrumb: "Este Computador > home > documents" */
    {
        font_draw_string_to_buffer_scaled(win->buffer, w, h, nav_x, nav_text_y, "Este Computador", col_text_grey, bg_navbar, s);
        nav_x += 15 * 8 * s + 4 * s;
        
        if (kstrcmp(data->current_path, "/") != 0) {
            /* Parse path components */
            const char *p = data->current_path + 1; /* skip leading / */
            while (*p) {
                font_draw_string_to_buffer_scaled(win->buffer, w, h, nav_x, nav_text_y, ">", col_text_dim, bg_navbar, s);
                nav_x += 12 * s;
                
                char component[64];
                int ci = 0;
                while (*p && *p != '/' && ci < 63) {
                    component[ci++] = *p++;
                }
                component[ci] = '\0';
                if (*p == '/') p++;
                
                font_draw_string_to_buffer_scaled(win->buffer, w, h, nav_x, nav_text_y, component, col_text_white, bg_navbar, s);
                nav_x += ci * 8 * s + 4 * s;
            }
        }
    }
    
    /* Search icon on right */
    draw_icon_to_buffer(win->buffer, w, h, w - (40 * s), (navbar_h - 32 * s) / 2, icon_search, ICON_SEARCH_WIDTH, ICON_SEARCH_HEIGHT);
    
    /* ============================================================ */
    /* 3. Sidebar: Quick Access + This PC                            */
    /* ============================================================ */
    int32_t sy = navbar_h + 12 * s; /* current Y for sidebar items */
    int32_t sx = 8 * s;             /* left padding */
    int32_t item_height = 28 * s;   /* height of each sidebar item */
    
    /* --- Quick Access Header --- */
    if (data->qa_open)
        draw_chevron_down(win->buffer, w, h, sx + 2*s, sy + 5*s, col_text_dim);
    else
        draw_chevron_right(win->buffer, w, h, sx + 2*s, sy + 4*s, col_text_dim);
    
    font_draw_string_to_buffer_scaled(win->buffer, w, h, sx + 14 * s, sy + 2 * s, "Acesso Rapido", col_text_grey, bg_sidebar, s);
    sy += item_height;
    
    if (data->qa_open) {
        /* Quick Access items */
        struct { const char *label; const char *path; int id; } qa_items[] = {
            { "Home",       "/home",            0 },
            { "Documentos", "/home/documents",  1 },
            { "Imagens",    "/home/pictures",    2 },
        };
        
        for (int i = 0; i < 3; i++) {
            /* Highlight if selected or if current path matches */
            bool is_sel = (data->selected_sidebar == qa_items[i].id);
            bool is_cur = (kstrcmp(data->current_path, qa_items[i].path) == 0);
            
            if (is_sel || is_cur) {
                draw_rounded_rect_buffer(win->buffer, w, h, sx, sy, sidebar_w - 16 * s, item_height - 2 * s, 6 * s, col_highlight);
            }
            
            /* Small folder icon */
            draw_mini_folder(win->buffer, w, h, sx + 10 * s, sy + 4 * s, 0x00F5A623);
            
            /* Label */
            uint32_t lcol = (is_sel || is_cur) ? col_text_white : col_text_grey;
            font_draw_string_to_buffer_scaled(win->buffer, w, h, sx + 28 * s, sy + 4 * s, qa_items[i].label, lcol, (is_sel || is_cur) ? col_highlight : bg_sidebar, s);
            
            sy += item_height;
        }
    }
    
    /* Separator line */
    sy += 4 * s;
    draw_hline_buffer(win->buffer, w, h, sx, sy, sidebar_w - 16 * s, col_separator);
    sy += 8 * s;
    
    /* --- This PC Header --- */
    if (data->pc_open)
        draw_chevron_down(win->buffer, w, h, sx + 2*s, sy + 5*s, col_text_dim);
    else
        draw_chevron_right(win->buffer, w, h, sx + 2*s, sy + 4*s, col_text_dim);
    
    font_draw_string_to_buffer_scaled(win->buffer, w, h, sx + 14 * s, sy + 2 * s, "Este Computador", col_text_grey, bg_sidebar, s);
    sy += item_height;
    
    if (data->pc_open) {
        /* Disk C: entry */
        bool is_disk_sel = (data->selected_sidebar == 10);
        bool is_disk_cur = (kstrcmp(data->current_path, "/") == 0);
        
        if (is_disk_sel || is_disk_cur) {
            draw_rounded_rect_buffer(win->buffer, w, h, sx, sy, sidebar_w - 16 * s, 68 * s, 6 * s, col_highlight);
        }
        
        /* Disk icon */
        draw_mini_disk(win->buffer, w, h, sx + 10 * s, sy + 4 * s, 0x006688AA);
        
        /* Disk name */
        uint32_t disk_bg = (is_disk_sel || is_disk_cur) ? col_highlight : bg_sidebar;
        font_draw_string_to_buffer_scaled(win->buffer, w, h, sx + 28 * s, sy + 4 * s, "Genesi (C:)", col_text_white, disk_bg, s);
        
        /* Progress bar: 120 GB total, 80 GB free => 40 GB used => 33% used */
        int32_t bar_x = sx + 10 * s;
        int32_t bar_y = sy + 24 * s;
        int32_t bar_w = sidebar_w - 36 * s;
        int32_t bar_h = 10 * s;
        int32_t used_pct = 33; /* 40/120 ~ 33% */
        int32_t used_w = (bar_w * used_pct) / 100;
        
        /* Bar background (empty) */
        draw_rounded_rect_buffer(win->buffer, w, h, bar_x, bar_y, bar_w, bar_h, 3 * s, 0x003A3C40);
        /* Bar fill (used space) */
        if (used_w > 0)
            draw_rounded_rect_buffer(win->buffer, w, h, bar_x, bar_y, used_w, bar_h, 3 * s, col_accent);
        
        /* Space text */
        font_draw_string_to_buffer_scaled(win->buffer, w, h, sx + 10 * s, sy + 40 * s, "80 GB livre de 120 GB", col_text_dim, disk_bg, s);
        
        sy += 72 * s;
    }
    
    /* ============================================================ */
    /* 4. Content area: file/folder grid                             */
    /* ============================================================ */
    struct vfs_node *dir = vfs_find(data->current_path);
    
    int32_t content_x = sidebar_w + 16 * s;
    int32_t content_y = navbar_h + 12 * s;
    int32_t content_w = w - sidebar_w - 16 * s;
    int32_t content_h = h - navbar_h - statusbar_h;
    
    if (!dir || dir->type != 2) {
        font_draw_string_to_buffer_scaled(win->buffer, w, h, content_x, content_y + 20 * s, "Diretorio nao encontrado.", 0x00FF4444, bg_content, s);
    } else {
        int32_t grid_item_w = 110 * s;
        int32_t grid_item_h = 120 * s;
        int32_t box_size = 72 * s;
        
        int cols = content_w / grid_item_w;
        if (cols < 1) cols = 1;
        
        for (int i = 0; i < dir->num_children; i++) {
            struct vfs_node *child = dir->children[i];
            int col = i % cols;
            int row = i / cols;
            
            int32_t cx = content_x + col * grid_item_w;
            int32_t cy = content_y + row * grid_item_h - data->scroll_y;
            if (cy + grid_item_h < (int32_t)navbar_h || cy > (int32_t)(h - statusbar_h)) continue;
            
            const uint32_t *icon_ptr = (child->type == 2) ? icon_folder : icon_doc;
            uint32_t icon_width = (child->type == 2) ? ICON_FOLDER_WIDTH : ICON_DOC_WIDTH;
            uint32_t icon_height = (child->type == 2) ? ICON_FOLDER_HEIGHT : ICON_DOC_HEIGHT;
            uint32_t box_color = (child->type == 2) ? 0x002D3038 : 0x00282C30;
            
            if (child->type != 2) {
                int nlen = kstrlen(child->name);
                if (nlen >= 4 && kstrcmp(child->name + nlen - 4, ".bmp") == 0) {
                    icon_ptr = icon_image;
                    icon_width = ICON_IMAGE_WIDTH;
                    icon_height = ICON_IMAGE_HEIGHT;
                } else if (nlen >= 4 && kstrcmp(child->name + nlen - 4, ".mp4") == 0) {
                    icon_ptr = icon_video;
                    icon_width = ICON_VIDEO_WIDTH;
                    icon_height = ICON_VIDEO_HEIGHT;
                }
            }
            
            /* Draw rounded box */
            draw_rounded_rect_buffer(win->buffer, w, h, cx, cy, box_size, box_size, 12 * s, box_color);
            
            /* Draw icon */
            if (icon_ptr) {
                draw_icon_to_buffer(win->buffer, w, h, cx + (box_size - icon_width*s)/2, cy + (box_size - icon_height*s)/2, icon_ptr, icon_width, icon_height);
            }
            
            /* Draw text (truncate if too long) */
            char short_name[16];
            int len = kstrlen(child->name);
            if (len > 11) {
                kmemcpy(short_name, child->name, 9);
                short_name[9] = '.'; short_name[10] = '.'; short_name[11] = '\0';
            } else {
                kmemcpy(short_name, child->name, len + 1);
            }
            
            int32_t text_w = kstrlen(short_name) * 8 * s;
            int32_t text_x = cx + (box_size - text_w) / 2;
            font_draw_string_to_buffer_scaled(win->buffer, w, h, text_x, cy + box_size + 8 * s, short_name, col_text_grey, bg_content, s);
        }
        
        /* ============================================================ */
        /* 5. Status bar: item count                                     */
        /* ============================================================ */
        {
            char count_str[32];
            int nc = dir->num_children;
            int ci = 0;
            
            /* Simple itoa for small numbers */
            if (nc == 0) {
                count_str[ci++] = '0';
            } else {
                char tmp[10];
                int ti = 0;
                int n = nc;
                while (n > 0) { tmp[ti++] = '0' + (n % 10); n /= 10; }
                for (int j = ti - 1; j >= 0; j--) count_str[ci++] = tmp[j];
            }
            
            const char *suffix = " itens";
            for (int j = 0; suffix[j]; j++) count_str[ci++] = suffix[j];
            count_str[ci] = '\0';
            
            font_draw_string_to_buffer_scaled(win->buffer, w, h, sidebar_w + 12 * s, h - statusbar_h + 6 * s, count_str, col_text_dim, bg_status, s);
        }
    }
    
    /* Update title to reflect path */
    int t_len = 0;
    const char *prefix = "Files - ";
    while(prefix[t_len]) { win->title[t_len] = prefix[t_len]; t_len++; }
    int p_len = 0;
    while(data->current_path[p_len] && t_len < WINDOW_MAX_TITLE - 1) {
        win->title[t_len++] = data->current_path[p_len++];
    }
    win->title[t_len] = '\0';
}

static void explorer_on_resize(window_t *win) {
    explorer_draw_path(win);
}

static void explorer_navigate_to(window_t *win, const char *path) {
    struct explorer_data *data = (struct explorer_data *)win->user_data;
    int len = kstrlen(path);
    if (len >= 255) len = 255;
    kmemcpy(data->current_path, path, len);
    data->current_path[len] = '\0';
    data->scroll_y = 0;
    explorer_draw_path(win);
}

static void explorer_go_up(window_t *win) {
    struct explorer_data *data = (struct explorer_data *)win->user_data;
    if (kstrcmp(data->current_path, "/") == 0) return;
    
    int last_slash = -1;
    int len = kstrlen(data->current_path);
    for (int i = len - 1; i >= 0; i--) {
        if (data->current_path[i] == '/') {
            last_slash = i;
            break;
        }
    }
    if (last_slash == 0) {
        data->current_path[0] = '/';
        data->current_path[1] = '\0';
    } else if (last_slash > 0) {
        data->current_path[last_slash] = '\0';
    }
    data->scroll_y = 0;
    explorer_draw_path(win);
}

static void explorer_on_mouse(window_t *win, int32_t mx, int32_t my, bool mdown) {
    if (!mdown) return;
    struct explorer_data *data = (struct explorer_data *)win->user_data;
    uint32_t w = win->width;
    uint32_t h = win->height;
    int s = g_ui_scale;
    
    int32_t sidebar_w = data->sidebar_width;
    int32_t navbar_h = 48 * s;
    int32_t statusbar_h = 28 * s;
    
    /* ---- Click on navbar ---- */
    if (my < navbar_h) {
        /* Back button (first ~28px) */
        if (mx < 28 * s) {
            explorer_go_up(win);
            return;
        }
        /* Ignore other navbar clicks for now */
        return;
    }
    
    /* ---- Click on sidebar ---- */
    if (mx < sidebar_w && my >= navbar_h && my < (int32_t)(h - statusbar_h)) {
        int32_t sy = navbar_h + 12 * s;
        int32_t item_height = 28 * s;
        int32_t sx = 8 * s;
        
        /* Quick Access header */
        if (my >= sy && my < sy + item_height) {
            data->qa_open = !data->qa_open;
            explorer_draw_path(win);
            return;
        }
        sy += item_height;
        
        if (data->qa_open) {
            /* QA items */
            const char *qa_paths[] = { "/home", "/home/documents", "/home/pictures" };
            for (int i = 0; i < 3; i++) {
                if (my >= sy && my < sy + item_height) {
                    data->selected_sidebar = i;
                    explorer_navigate_to(win, qa_paths[i]);
                    return;
                }
                sy += item_height;
            }
        }
        
        /* Skip separator space */
        sy += 12 * s;
        
        /* This PC header */
        if (my >= sy && my < sy + item_height) {
            data->pc_open = !data->pc_open;
            explorer_draw_path(win);
            return;
        }
        sy += item_height;
        
        if (data->pc_open) {
            /* Disk C: item (68*s tall) */
            if (my >= sy && my < sy + 68 * s) {
                data->selected_sidebar = 10;
                explorer_navigate_to(win, "/");
                return;
            }
        }
        
        return;
    }
    
    /* ---- Click on content area ---- */
    if (mx >= sidebar_w && my >= navbar_h && my < (int32_t)(h - statusbar_h)) {
        struct vfs_node *dir = vfs_find(data->current_path);
        if (!dir || dir->type != 2) return;
        
        int32_t content_x = sidebar_w + 16 * s;
        int32_t content_y = navbar_h + 12 * s;
        int32_t content_w = w - sidebar_w - 16 * s;
        
        int32_t grid_item_w = 110 * s;
        int32_t grid_item_h = 120 * s;
        int32_t box_size = 72 * s;
        
        int cols = content_w / grid_item_w;
        if (cols < 1) cols = 1;
        
        for (int i = 0; i < dir->num_children; i++) {
            int col = i % cols;
            int row = i / cols;
            
            int32_t cx = content_x + col * grid_item_w;
            int32_t cy = content_y + row * grid_item_h - data->scroll_y;
            
            if (mx >= cx && mx <= cx + box_size && my >= cy && my <= cy + box_size) {
                struct vfs_node *child = dir->children[i];
                if (child->type == 2) {
                    /* Navigate into directory */
                    int len = kstrlen(data->current_path);
                    if (data->current_path[len-1] != '/') {
                        data->current_path[len] = '/';
                        len++;
                    }
                    int clen = kstrlen(child->name);
                    kmemcpy(data->current_path + len, child->name, clen + 1);
                    data->scroll_y = 0;
                    data->selected_sidebar = -1;
                    explorer_draw_path(win);
                } else {
                    kprintf("  [Explorer] Clicked file: %s\n", child->name);
                }
                return;
            }
        }
    }
}

void desktop_create_explorer(void) {
    uint32_t w = 820 * g_ui_scale;
    uint32_t h = 560 * g_ui_scale;
    window_t *win = wm_create_window(200 * g_ui_scale, 120 * g_ui_scale, w, h, "Files - /");
    if (win && win->buffer) {
        extern void *kmalloc(size_t size);
        struct explorer_data *data = (struct explorer_data *)kmalloc(sizeof(struct explorer_data));
        kmemset(data, 0, sizeof(struct explorer_data));
        data->current_path[0] = '/';
        data->current_path[1] = '\0';
        data->sidebar_width = 200 * g_ui_scale;
        data->selected_sidebar = -1;
        data->qa_open = true;
        data->pc_open = true;
        win->user_data = data;
        
        win->on_resize = explorer_on_resize;
        win->on_mouse = explorer_on_mouse;
        explorer_draw_path(win);
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
    compositor_update(true);
}
