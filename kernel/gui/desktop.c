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
};

static void explorer_draw_path(window_t *win) {
    if (!win || !win->buffer) return;
    struct explorer_data *data = (struct explorer_data *)win->user_data;
    uint32_t w = win->width;
    uint32_t h = win->height;
    
    /* Dark mode background */
    for (uint32_t i = 0; i < w * h; i++) {
        win->buffer[i] = 0x001C1E23;
    }
    
    /* Search Icon in Header */
    draw_icon_to_buffer(win->buffer, w, h, w - (48 * g_ui_scale), 12 * g_ui_scale, icon_search, ICON_SEARCH_WIDTH, ICON_SEARCH_HEIGHT);
    
    /* Current Path text */
    font_draw_string_to_buffer_scaled(win->buffer, w, h, 20 * g_ui_scale, 20 * g_ui_scale, data->current_path, 0x00FFFFFF, 0x001C1E23, g_ui_scale);
    
    /* Back Button if not root */
    if (kstrcmp(data->current_path, "/") != 0) {
        draw_rounded_rect_buffer(win->buffer, w, h, 20 * g_ui_scale, 60 * g_ui_scale, 100 * g_ui_scale, 32 * g_ui_scale, 8 * g_ui_scale, 0x003B4455);
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 40 * g_ui_scale, 70 * g_ui_scale, "<- Back", 0x00FFFFFF, 0x003B4455, g_ui_scale);
    }
    
    /* Fetch VFS Node */
    struct vfs_node *dir = vfs_find(data->current_path);
    if (!dir || dir->type != 2) { /* VFS_DIRECTORY = 2 */
        font_draw_string_to_buffer_scaled(win->buffer, w, h, 40 * g_ui_scale, 120 * g_ui_scale, "Directory not found or is a file.", 0x00FF4444, 0x001C1E23, g_ui_scale);
        return;
    }
    
    /* Grid properties */
    int32_t start_x = 40 * g_ui_scale;
    int32_t start_y = 120 * g_ui_scale;
    int32_t item_w = 120 * g_ui_scale;
    int32_t item_h = 140 * g_ui_scale;
    int32_t box_size = 80 * g_ui_scale;
    
    int cols = (w - start_x) / item_w;
    if (cols < 1) cols = 1;
    
    for (int i = 0; i < dir->num_children; i++) {
        struct vfs_node *child = dir->children[i];
        int col = i % cols;
        int row = i / cols;
        
        int32_t cx = start_x + col * item_w;
        int32_t cy = start_y + row * item_h - data->scroll_y;
        if (cy + item_h < 0 || cy > (int32_t)h) continue;
        
        const uint32_t *icon_ptr = (child->type == 2) ? icon_folder : icon_doc;
        uint32_t icon_width = (child->type == 2) ? ICON_FOLDER_WIDTH : ICON_DOC_WIDTH;
        uint32_t icon_height = (child->type == 2) ? ICON_FOLDER_HEIGHT : ICON_DOC_HEIGHT;
        uint32_t box_color = (child->type == 2) ? 0x003B4455 : 0x002A3F3F;
        
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
        draw_rounded_rect_buffer(win->buffer, w, h, cx, cy, box_size, box_size, 16 * g_ui_scale, box_color);
        
        /* Draw icon */
        if (icon_ptr) {
            draw_icon_to_buffer(win->buffer, w, h, cx + (box_size - icon_width*g_ui_scale)/2, cy + (box_size - icon_height*g_ui_scale)/2, icon_ptr, icon_width, icon_height);
        }
        
        /* Draw text (truncate if too long) */
        char short_name[16];
        int len = kstrlen(child->name);
        if (len > 12) {
            kmemcpy(short_name, child->name, 10);
            short_name[10] = '.'; short_name[11] = '.'; short_name[12] = '\0';
        } else {
            kmemcpy(short_name, child->name, len + 1);
        }
        
        font_draw_string_to_buffer_scaled(win->buffer, w, h, cx + (box_size - kstrlen(short_name)*8*g_ui_scale)/2, cy + box_size + 12*g_ui_scale, short_name, 0x00A0A0A0, 0x001C1E23, g_ui_scale);
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

static void explorer_on_mouse(window_t *win, int32_t mx, int32_t my, bool mdown) {
    if (!mdown) return;
    struct explorer_data *data = (struct explorer_data *)win->user_data;
    
    /* Back button clicked? */
    if (kstrcmp(data->current_path, "/") != 0) {
        if (mx >= 20 * g_ui_scale && mx <= 120 * g_ui_scale && my >= 60 * g_ui_scale && my <= 92 * g_ui_scale) {
            /* Go up one directory */
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
            return;
        }
    }
    
    /* Clicked a folder? */
    struct vfs_node *dir = vfs_find(data->current_path);
    if (!dir || dir->type != 2) return;
    
    int32_t start_x = 40 * g_ui_scale;
    int32_t start_y = 120 * g_ui_scale;
    int32_t item_w = 120 * g_ui_scale;
    int32_t item_h = 140 * g_ui_scale;
    int32_t box_size = 80 * g_ui_scale;
    
    int cols = (win->width - start_x) / item_w;
    if (cols < 1) cols = 1;
    
    for (int i = 0; i < dir->num_children; i++) {
        int col = i % cols;
        int row = i / cols;
        
        int32_t cx = start_x + col * item_w;
        int32_t cy = start_y + row * item_h - data->scroll_y;
        
        if (mx >= cx && mx <= cx + box_size && my >= cy && my <= cy + box_size) {
            struct vfs_node *child = dir->children[i];
            if (child->type == 2) { /* Directory */
                /* Append to path */
                int len = kstrlen(data->current_path);
                if (data->current_path[len-1] != '/') {
                    data->current_path[len] = '/';
                    len++;
                }
                int clen = kstrlen(child->name);
                kmemcpy(data->current_path + len, child->name, clen + 1);
                data->scroll_y = 0;
                explorer_draw_path(win);
            } else {
                /* File clicked - open in a basic editor/viewer? Or just log */
                kprintf("  [Explorer] Clicked file: %s\n", child->name);
            }
            return;
        }
    }
}

void desktop_create_explorer(void) {
    uint32_t w = 680 * g_ui_scale;
    uint32_t h = 500 * g_ui_scale;
    window_t *win = wm_create_window(250 * g_ui_scale, 150 * g_ui_scale, w, h, "Files - /");
    if (win && win->buffer) {
        extern void *kmalloc(size_t size);
        struct explorer_data *data = (struct explorer_data *)kmalloc(sizeof(struct explorer_data));
        kmemset(data, 0, sizeof(struct explorer_data));
        data->current_path[0] = '/';
        data->current_path[1] = '\0';
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
