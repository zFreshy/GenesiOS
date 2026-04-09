/*
 * kernel/gui/compositor.c
 * Renders all windows and the mouse cursor to a backbuffer, then blits.
 */
#include "compositor.h"
#include "window.h"
#include "../gfx/framebuffer.h"
#include "../gfx/font.h"
#include "../drivers/mouse.h"
#include "../include/kprintf.h"
#include "icon_start.h"

static uint32_t  s_width      = 0;
static uint32_t  s_height     = 0;
int g_ui_scale = 1;
int g_current_wallpaper = 1; /* 0 = Gradient, 1 = Image */
uint32_t *g_wallpaper_raw = NULL;

static window_t *s_drag_win = NULL;
static int32_t s_drag_off_x = 0;
static int32_t s_drag_off_y = 0;
static bool s_mouse_was_down = false;

static window_t *s_resizing_win = NULL;
static int s_resize_dir = 0;
static int32_t s_resize_start_w = 0;
static int32_t s_resize_start_h = 0;
static int32_t s_resize_start_mx = 0;
static int32_t s_resize_start_my = 0;
static bool      s_start_menu_open = false;

void compositor_render(void);

static void fb_clear_buffer(uint32_t *bb, uint32_t color) {
    extern void *kmemset32(void *dst, uint32_t val, size_t count32);
    uint32_t total = s_width * s_height;
    kmemset32(bb, color, total);
}

/* ------------------------------------------------------------------ */
/* Initialize the compositor                                          */
/* ------------------------------------------------------------------ */
void compositor_init(void) {
    if (!fb_available()) return;
    s_width  = fb_width();
    s_height = fb_height();
    
    /* Sempre usar escala 1 para evitar lentidão e tamanhos gigantes */
    g_ui_scale = 1;
    
    /* Find wallpaper module */
    extern uint64_t g_mboot_info;
    #include "../include/multiboot2.h"
    mb2_info_t *info = (mb2_info_t *)(uintptr_t)g_mboot_info;
    if (info) {
        mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)info + 8);
        while (tag->type != MB2_TAG_END) {
            if (tag->type == MB2_TAG_MODULE) {
                mb2_module_tag_t *mod = (mb2_module_tag_t *)tag;
                /* Find "wallpaper" anywhere in the string */
                const char *s = mod->string;
                bool found = false;
                for (int i = 0; s[i] != '\0'; i++) {
                    if (s[i] == 'w' && s[i+1] == 'a' && s[i+2] == 'l' && s[i+3] == 'l' && 
                        s[i+4] == 'p' && s[i+5] == 'a' && s[i+6] == 'p' && s[i+7] == 'e' && s[i+8] == 'r') {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    g_wallpaper_raw = (uint32_t *)(uintptr_t)mod->mod_start;
                    kprintf("Compositor: Found wallpaper module at %p\n", g_wallpaper_raw);
                }
            }
            tag = (mb2_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw Wallpaper (Aladin OS Style Gradient)                          */
/* ------------------------------------------------------------------ */
static void draw_wallpaper(void) {
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb) return;

    if (g_current_wallpaper == 0 || !g_wallpaper_raw) {
        /* Gradient Wallpaper */
        for (uint32_t y = 0; y < s_height; y++) {
            uint32_t t = (y * 255) / s_height;
            uint32_t r = 0xE6 + (t * (0x90 - 0xE6)) / 255;
            uint32_t g = 0xE6 + (t * (0xA0 - 0xE6)) / 255;
            uint32_t b = 0xFA + (t * (0xE0 - 0xFA)) / 255;
            
            uint32_t color = (r << 16) | (g << 8) | b;
            
            for (uint32_t x = 0; x < s_width; x++) {
                bb[y * fb_pitch_words() + x] = color;
            }
        }
    } else {
        /* Image Wallpaper (Direct from 1080p raw module) */
        /* It is guaranteed to be 1920x1080 ARGB */
        uint32_t w_w = 1920;
        uint32_t w_h = 1080;
        
        if (s_width == w_w && s_height == w_h) {
            /* Exact match, just copy row by row to respect pitch */
            for (uint32_t y = 0; y < w_h; y++) {
                kmemcpy(&bb[y * fb_pitch_words()], &g_wallpaper_raw[y * w_w], w_w * 4);
            }
        } else {
            /* Center or crop the wallpaper instead of slow per-pixel scaling */
            uint32_t copy_w = (s_width < w_w) ? s_width : w_w;
            uint32_t copy_h = (s_height < w_h) ? s_height : w_h;
            
            /* Fill background with black first in case screen is larger */
            if (s_width > w_w || s_height > w_h) {
                fb_clear_buffer(bb, 0xFF000000);
            }
            
            for (uint32_t y = 0; y < copy_h; y++) {
                kmemcpy(&bb[y * fb_pitch_words()], &g_wallpaper_raw[y * w_w], copy_w * 4);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Fast Integer Square Root for Anti-Aliasing                         */
/* ------------------------------------------------------------------ */
static uint32_t fast_sqrt(uint32_t n) {
    uint32_t root = 0;
    uint32_t bit = 1UL << 30;
    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

/* ------------------------------------------------------------------ */
/* Draw Rounded Rectangle (Filled with Anti-Aliasing)                 */
/* ------------------------------------------------------------------ */
static void draw_rounded_rect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color, bool blend) {
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb) return;

    uint32_t base_alpha = (color >> 24) & 0xFF;
    uint32_t cr = (color >> 16) & 0xFF;
    uint32_t cg = (color >> 8) & 0xFF;
    uint32_t cb = color & 0xFF;

    for (int32_t cy = y; cy < y + h; cy++) {
        if (cy < 0 || cy >= (int32_t)s_height) continue;
        
        /* If we are completely inside the rectangle vertically, and there is no blending, 
           we can fast fill the inner horizontal row */
        if (!blend && (cy >= y + r) && (cy < y + h - r)) {
            int32_t cx_start = (x < 0) ? 0 : x;
            int32_t cx_end = (x + w > (int32_t)s_width) ? (int32_t)s_width : x + w;
            if (cx_end > cx_start) {
                extern void *kmemset32(void *dst, uint32_t val, size_t count32);
                kmemset32(&bb[cy * fb_pitch_words() + cx_start], color & 0xFFFFFF, cx_end - cx_start);
            }
            continue;
        }
        
        for (int32_t cx = x; cx < x + w; cx++) {
            if (cx < 0 || cx >= (int32_t)s_width) continue;

            /* Distância até a quina mais próxima para desenhar o círculo */
            int32_t dx = 0, dy = 0;
            if (cx < x + r) dx = (x + r - 1) - cx;
            else if (cx >= x + w - r) dx = cx - (x + w - r);
            
            if (cy < y + r) dy = (y + r - 1) - cy;
            else if (cy >= y + h - r) dy = cy - (y + h - r);
            
            uint32_t alpha = base_alpha;
            if (dx > 0 || dy > 0) {
                uint32_t dist_sq = dx*dx + dy*dy;
                uint32_t rr = r*r;
                
                if (dist_sq >= rr) {
                    continue; /* Fora do círculo */
                } else if (dist_sq > (r - 2)*(r - 2)) {
                    /* Borda do círculo: aplica Anti-Aliasing (suavização) */
                    uint32_t dist = fast_sqrt(dist_sq);
                    if (dist >= (int32_t)r) continue;
                    /* Calcula opacidade baseada em quão perto do sub-pixel está (0 a 255) */
                    uint32_t edge_alpha = 255 - ((dist - (r - 2)) * 255 / 2);
                    alpha = (alpha * edge_alpha) / 255;
                }
            }

            if (blend && alpha < 255) {
                uint32_t bg = bb[cy * fb_pitch_words() + cx];
                uint32_t bgr = (bg >> 16) & 0xFF;
                uint32_t bgg = (bg >> 8) & 0xFF;
                uint32_t bgb = bg & 0xFF;
                uint32_t nr = (cr * alpha + bgr * (255 - alpha)) / 255;
                uint32_t ng = (cg * alpha + bgg * (255 - alpha)) / 255;
                uint32_t nb = (cb * alpha + bgb * (255 - alpha)) / 255;
                bb[cy * fb_pitch_words() + cx] = (nr << 16) | (ng << 8) | nb;
            } else {
                bb[cy * fb_pitch_words() + cx] = color & 0xFFFFFF;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw Drop Shadow (Fake Alpha Blending)                             */
/* ------------------------------------------------------------------ */
static void draw_shadow(int32_t x, int32_t y, int32_t w, int32_t h) {
    if (!fb_available()) return;
    
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb) return;

    int32_t shadow_offset = 6;
    int32_t sx = x + shadow_offset;
    int32_t sy = y + shadow_offset;
    
    for (int32_t cy = sy; cy < sy + h + 10; cy++) {
        if (cy < 0 || cy >= (int32_t)s_height) continue;
        for (int32_t cx = sx; cx < sx + w + 10; cx++) {
            if (cx < 0 || cx >= (int32_t)s_width) continue;
            
            /* Otimização drástica: Não desenhar a sombra DEBAIXO da janela, pois será sobrescrita */
            if (cx >= x && cx < x + w && cy >= y && cy < y + h) {
                continue;
            }
            
            /* Basic darken blend based on distance to make it softer */
            int32_t dist_x = 0, dist_y = 0;
            if (cx < sx + 10) dist_x = 10 - (cx - sx);
            else if (cx > sx + w) dist_x = cx - (sx + w);
            if (cy < sy + 10) dist_y = 10 - (cy - sy);
            else if (cy > sy + h) dist_y = cy - (sy + h);
            
            int32_t dist = dist_x > dist_y ? dist_x : dist_y;
            if (dist > 10) continue;
            
            uint32_t intensity = 10 - dist; /* 0 to 10 */
            
            uint32_t pitch_words = fb_pitch_words();
            uint32_t bg = bb[cy * pitch_words + cx];
            
            uint32_t r = ((bg >> 16) & 0xFF);
            uint32_t g = ((bg >> 8) & 0xFF);
            uint32_t b = (bg & 0xFF);
            
            /* Darken by up to 25% */
            r = r - (r * intensity / 40);
            g = g - (g * intensity / 40);
            b = b - (b * intensity / 40);
            
            bb[cy * pitch_words + cx] = (r << 16) | (g << 8) | b;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw a single window                                               */
/* ------------------------------------------------------------------ */
static void draw_window(window_t *win) {
    if (!win) return;
    if (win->is_minimized) return;

    int32_t title_h = 56 * g_ui_scale;
    int32_t win_radius = (win->is_maximized || win->is_fullscreen) ? 0 : 16 * g_ui_scale; /* Rounded corners */

    /* Draw window drop shadow */
    if (win->x > 0 && win->y > title_h && !win->is_maximized && !win->is_fullscreen) {
        draw_shadow(win->x, win->y - title_h, win->width, win->height + title_h);
    }

    /* Draw window title bar (semi-transparent light blur effect) */
    if (!win->is_fullscreen && win->y >= title_h) {
        int32_t title_y = win->y - title_h;
        
        /* Seamless theme: sample color from window buffer or default to dark */
        uint32_t tb_color = 0xFF1C1E23;
        if (win->buffer) {
            tb_color = win->buffer[0] | 0xFF000000; /* Ensure opaque */
        }

        /* Draw title bar background (rounded top) */
        draw_rounded_rect(win->x, title_y, win->width, title_h + win_radius, win_radius, tb_color, true);

        /* Control buttons (MacOS style traffic lights on the LEFT) */
        int32_t btn_s = 14 * g_ui_scale;
        int32_t btn_y = title_y + (title_h - btn_s) / 2;
        int32_t btn_r = 7 * g_ui_scale;
        
        /* Close (Red) */
        draw_rounded_rect(win->x + (20 * g_ui_scale), btn_y, btn_s, btn_s, btn_r, 0xFFEF4444, true);
        
        /* Minimize (Yellow) */
        draw_rounded_rect(win->x + (44 * g_ui_scale), btn_y, btn_s, btn_s, btn_r, 0xFFF5A623, true);
        
        /* Maximize (Green) */
        draw_rounded_rect(win->x + (68 * g_ui_scale), btn_y, btn_s, btn_s, btn_r, 0xFF10B981, true);

        /* Title text */
        uint32_t text_color = 0x00808080; /* Subtle grey text */
        font_draw_string_scaled(win->x + (100 * g_ui_scale), title_y + (12 * g_ui_scale), win->title, text_color, 0x00000000, g_ui_scale);
    }

    /* Draw window content buffer (rounded bottom) */
    if (win->buffer) {
        extern uint32_t *fb_get_backbuffer(void);
        uint32_t *bb = fb_get_backbuffer();
        if (!bb) return;

        int32_t buf_w = win->width;
        int32_t buf_h = win->height;
        
        for (int32_t cy = 0; cy < buf_h; cy++) {
            int32_t py = win->y + cy;
            if (py < 0 || py >= (int32_t)s_height) continue;
            
            /* If we are not in the rounded bottom corner area, we can fast-copy the whole row */
            if (win_radius == 0 || cy < buf_h - win_radius) {
                int32_t cx_start = 0;
                int32_t cx_end = buf_w;
                
                /* Clip horizontally */
                if (win->x < 0) cx_start = -win->x;
                if (win->x + buf_w > (int32_t)s_width) cx_end = s_width - win->x;
                
                if (cx_end > cx_start) {
                    kmemcpy(&bb[py * fb_pitch_words() + (win->x + cx_start)], 
                            &win->buffer[cy * buf_w + cx_start], 
                            (cx_end - cx_start) * 4);
                }
            } else {
                /* Bottom corners - draw pixel by pixel */
                for (int32_t cx = 0; cx < buf_w; cx++) {
                    int32_t px = win->x + cx;
                    if (px < 0 || px >= (int32_t)s_width) continue;
                    
                    int32_t dx = 0, dy = 0;
                    if (cx < win_radius) {
                        dx = (win_radius - 1) - cx;
                        dy = cy - (buf_h - win_radius);
                    } else if (cx >= buf_w - win_radius) {
                        dx = cx - (buf_w - win_radius);
                        dy = cy - (buf_h - win_radius);
                    }
                    
                    if (dx*dx + dy*dy >= win_radius * win_radius) continue;
                    
                    bb[py * fb_pitch_words() + px] = win->buffer[cy * buf_w + cx] | 0xFF000000;
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw an arrow cursor directly to framebuffer to avoid full redraw  */
/* ------------------------------------------------------------------ */
static int32_t s_old_cx = -1;
static int32_t s_old_cy = -1;

static void draw_cursor_direct(int32_t cx, int32_t cy) {
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb || !g_fb.buffer) return;

    int32_t cw = 16 * g_ui_scale;
    int32_t ch = 17 * g_ui_scale;

    /* 1. Restore old cursor area from backbuffer to screen */
    if (s_old_cx != -1 && s_old_cy != -1) {
        for (int32_t y = 0; y < ch; y++) {
            int32_t py = s_old_cy + y;
            if (py < 0 || py >= (int32_t)g_fb.height) continue;
            uint32_t *dst_row = &g_fb.buffer[py * (g_fb.pitch / 4)];
            uint32_t *src_row = &bb[py * (g_fb.pitch / 4)];
            for (int32_t x = 0; x < cw; x++) {
                int32_t px = s_old_cx + x;
                if (px < 0 || px >= (int32_t)g_fb.width) continue;
                dst_row[px] = src_row[px];
            }
        }
    }

    s_old_cx = cx;
    s_old_cy = cy;

    /* 2. Draw new cursor to screen directly */
    static const char *cursor_shape[17] = {
        "X               ",
        "XX              ",
        "X.X             ",
        "X..X            ",
        "X...X           ",
        "X....X          ",
        "X.....X         ",
        "X......X        ",
        "X.......X       ",
        "X........X      ",
        "X.....XXXXX     ",
        "X..X..X         ",
        "X.X X..X        ",
        "XX  X..X        ",
        "X    X..X       ",
        "     X..X       ",
        "      XX        "
    };

    for (int y = 0; y < 17; y++) {
        for (int x = 0; x < 16; x++) {
            char c = cursor_shape[y][x];
            if (c != ' ') {
                for (int sy = 0; sy < g_ui_scale; sy++) {
                    for (int sx = 0; sx < g_ui_scale; sx++) {
                        int32_t px = cx + x * g_ui_scale + sx;
                        int32_t py = cy + y * g_ui_scale + sy;
                        if (px < 0 || px >= (int32_t)g_fb.width || py < 0 || py >= (int32_t)g_fb.height) continue;
                        
                        if (c == 'X') {
                            g_fb.buffer[py * (g_fb.pitch / 4) + px] = 0x00000000;
                        } else if (c == '.') {
                            g_fb.buffer[py * (g_fb.pitch / 4) + px] = 0x00FFFFFF;
                        }
                    }
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw top bar and widgets (Aladin OS Style)                         */
/* ------------------------------------------------------------------ */

static uint8_t get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static uint8_t bcd2bin(uint8_t bcd) {
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0xf);
}

static void draw_desktop_widgets(void) {
    /* Read RTC time */
    uint8_t s = get_rtc_register(0x00);
    uint8_t m = get_rtc_register(0x02);
    uint8_t h = get_rtc_register(0x04);
    
    /* Read RTC date */
    uint8_t day = get_rtc_register(0x07);
    uint8_t month = get_rtc_register(0x08);
    // uint8_t year = get_rtc_register(0x09);
    
    uint8_t regB = get_rtc_register(0x0B);
    
    if (!(regB & 0x04)) {
        s = bcd2bin(s);
        m = bcd2bin(m);
        h = ((h & 0x0F) + (((h & 0x70) / 16) * 10)) | (h & 0x80);
        day = bcd2bin(day);
        month = bcd2bin(month);
    }
    
    /* Convert 12 hour clock to 24 hour */
    if (!(regB & 0x02) && (h & 0x80)) {
        h = ((h & 0x7F) + 12) % 24;
    }
    
    char time_str[16];
    time_str[0] = '0' + (h / 10);
    time_str[1] = '0' + (h % 10);
    time_str[2] = ':';
    time_str[3] = '0' + (m / 10);
    time_str[4] = '0' + (m % 10);
    time_str[5] = '\0';
    
    const char *months[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char *m_str = (month >= 1 && month <= 12) ? months[month] : "???";
    
    char date_str[32];
    date_str[0] = m_str[0];
    date_str[1] = m_str[1];
    date_str[2] = m_str[2];
    date_str[3] = ' ';
    date_str[4] = '0' + (day / 10);
    date_str[5] = '0' + (day % 10);
    date_str[6] = '\0';
    
    /* Huge Clock at Top Center */
    uint32_t clock_x = (s_width - (5 * 16 * g_ui_scale)) / 2; /* 5 chars */
    font_draw_string_scaled(clock_x + 2 * g_ui_scale, 42 * g_ui_scale, time_str, 0x005555AA, 0x00000000, g_ui_scale); /* Drop shadow */
    font_draw_string_scaled(clock_x, 40 * g_ui_scale, time_str, 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Date below clock */
    uint32_t date_x = (s_width - (6 * 16 * g_ui_scale)) / 2; /* 6 chars */
    font_draw_string_scaled(date_x, 80 * g_ui_scale, date_str, 0x00D0D0E0, 0x00000000, g_ui_scale);
}

/* ------------------------------------------------------------------ */
/* Draw taskbar (Floating Dock)                                       */
/* ------------------------------------------------------------------ */
static void apply_blur_rounded_rect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t blur_r, int32_t border_r) {
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb) return;
    
    int32_t x0 = x < 0 ? 0 : x;
    int32_t y0 = y < 0 ? 0 : y;
    int32_t x1 = x + w > (int32_t)s_width ? (int32_t)s_width : x + w;
    int32_t y1 = y + h > (int32_t)s_height ? (int32_t)s_height : y + h;
    
    int32_t bw = x1 - x0;
    int32_t bh = y1 - y0;
    if (bw <= 0 || bh <= 0) return;

    /* A simple fast 2-pass box blur */
    extern void *kmalloc(size_t size);
    uint32_t *temp = (uint32_t*)kmalloc(bw * bh * sizeof(uint32_t));
    if (!temp) return;
    
    /* Horizontal pass */
    for (int32_t cy = 0; cy < bh; cy++) {
        for (int32_t cx = 0; cx < bw; cx++) {
            uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
            int32_t count = 0;
            for (int32_t k = -blur_r; k <= blur_r; k++) {
                int32_t px = cx + k;
                if (px >= 0 && px < bw) {
                    uint32_t color = bb[(y0 + cy) * fb_pitch_words() + (x0 + px)];
                    sum_r += (color >> 16) & 0xFF;
                    sum_g += (color >> 8) & 0xFF;
                    sum_b += color & 0xFF;
                    count++;
                }
            }
            temp[cy * bw + cx] = ((sum_r / count) << 16) | ((sum_g / count) << 8) | (sum_b / count);
        }
    }
    
    /* Vertical pass and darken */
    for (int32_t cx = 0; cx < bw; cx++) {
        for (int32_t cy = 0; cy < bh; cy++) {
            int32_t px = x0 + cx;
            int32_t py = y0 + cy;
            
            /* Clip to rounded corners */
            int32_t dx = 0, dy = 0;
            if (px < x + border_r) dx = (x + border_r - 1) - px;
            else if (px >= x + w - border_r) dx = px - (x + w - border_r);
            if (py < y + border_r) dy = (y + border_r - 1) - py;
            else if (py >= y + h - border_r) dy = py - (y + h - border_r);
            
            if (dx*dx + dy*dy >= border_r * border_r) continue;
            
            uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
            int32_t count = 0;
            for (int32_t k = -blur_r; k <= blur_r; k++) {
                int32_t py_k = cy + k;
                if (py_k >= 0 && py_k < bh) {
                    uint32_t color = temp[py_k * bw + cx];
                    sum_r += (color >> 16) & 0xFF;
                    sum_g += (color >> 8) & 0xFF;
                    sum_b += color & 0xFF;
                    count++;
                }
            }
            
            /* Apply darkened translucent effect over the blur (Glassmorphism) */
            uint32_t br = sum_r / count;
            uint32_t bg = sum_g / count;
            uint32_t bb_col = sum_b / count;
            
            /* Blend with 0x1A1C23 at 50% opacity */
            uint32_t tint_r = 0x1A, tint_g = 0x1C, tint_b = 0x23;
            uint32_t fr = (tint_r * 128 + br * 128) / 256;
            uint32_t fg = (tint_g * 128 + bg * 128) / 256;
            uint32_t fb = (tint_b * 128 + bb_col * 128) / 256;
            
            bb[py * fb_pitch_words() + px] = (fr << 16) | (fg << 8) | fb;
        }
    }
    
    extern void kfree(void *ptr);
    kfree(temp);
}

static void draw_icon(int32_t x, int32_t y, const uint32_t *icon_data, uint32_t icon_w, uint32_t icon_h) {
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb) return;
    
    int32_t scaled_w = icon_w * g_ui_scale;
    int32_t scaled_h = icon_h * g_ui_scale;
    
    for (int32_t iy = 0; iy < scaled_h; iy++) {
        for (int32_t ix = 0; ix < scaled_w; ix++) {
            if (x + ix < 0 || x + ix >= (int32_t)s_width || y + iy < 0 || y + iy >= (int32_t)s_height) continue;
            
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
                /* For white tinted SVG icons, we just use alpha directly */
                uint32_t fr = 255, fg = 255, fb = 255;
                
                if (alpha == 255) {
                    bb[(y + iy) * fb_pitch_words() + (x + ix)] = (fr << 16) | (fg << 8) | fb;
                } else {
                    uint32_t bg = bb[(y + iy) * fb_pitch_words() + (x + ix)];
                    uint32_t bgr = (bg >> 16) & 0xFF;
                    uint32_t bgg = (bg >> 8) & 0xFF;
                    uint32_t bgb = bg & 0xFF;
                    
                    uint32_t r = (fr * alpha + bgr * (255 - alpha)) / 255;
                    uint32_t g = (fg * alpha + bgg * (255 - alpha)) / 255;
                    uint32_t b = (fb * alpha + bgb * (255 - alpha)) / 255;
                    
                    bb[(y + iy) * fb_pitch_words() + (x + ix)] = (r << 16) | (g << 8) | b;
                }
            }
        }
    }
}

#include "icons/icon_grid4.h"
#include "icons/icon_grid9.h"
#include "icons/icon_search.h"
#include "icons/icon_folder.h"
#include "icons/icon_cmd.h"
#include "icons/icon_power.h"

static void draw_taskbar(void) {
    int32_t tb_height = 64 * g_ui_scale;
    int32_t tb_width = 460 * g_ui_scale;
    int32_t tb_x = (s_width - tb_width) / 2;
    int32_t tb_y = s_height - tb_height - (20 * g_ui_scale); /* Floating */
    
    /* Taskbar background (Dark frosted glass, matching design) */
    apply_blur_rounded_rect(tb_x, tb_y, tb_width, tb_height, 6 * g_ui_scale, 32 * g_ui_scale);
    draw_rounded_rect(tb_x, tb_y, tb_width, tb_height, 32 * g_ui_scale, 0x602A2E33, true);
    
    /* Calculate icon spacing */
    int32_t item_count = 6;
    int32_t icon_size = 32 * g_ui_scale;
    int32_t padding = (tb_width - (item_count * icon_size)) / (item_count + 1);
    
    int32_t cur_x = tb_x + padding;
    int32_t icon_y = tb_y + (tb_height - icon_size) / 2;
    
    /* Find currently active app to highlight its icon */
    window_t *top_win = wm_get_top();
    int active_icon = -1;
    if (top_win) {
        if (kstrcmp(top_win->title, "Files / root / documents") == 0) {
            active_icon = 3; /* Folder icon index (0-based) */
        } else if (kstrcmp(top_win->title, "root@genesi-os: ~/dev/neural-core") == 0 || kstrcmp(top_win->title, "Terminal") == 0) {
            active_icon = 4; /* CMD/Terminal icon index */
        }
    }
    
    int32_t circle_size = 48 * g_ui_scale;
    
    /* 1. Grid 4 (Start Menu) */
    if (active_icon == 0) draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, 16 * g_ui_scale, 0x30FFFFFF, true);
    draw_icon(cur_x, icon_y, icon_grid4, ICON_GRID4_WIDTH, ICON_GRID4_HEIGHT);
    cur_x += icon_size + padding;
    
    /* 2. Grid 9 (App Drawer) */
    if (active_icon == 1) draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, 16 * g_ui_scale, 0x30FFFFFF, true);
    draw_icon(cur_x, icon_y, icon_grid9, ICON_GRID9_WIDTH, ICON_GRID9_HEIGHT);
    cur_x += icon_size + padding;
    
    /* 3. Search */
    if (active_icon == 2) draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, 16 * g_ui_scale, 0x30FFFFFF, true);
    draw_icon(cur_x, icon_y, icon_search, ICON_SEARCH_WIDTH, ICON_SEARCH_HEIGHT);
    cur_x += icon_size + padding;
    
    /* 4. Folder */
    if (active_icon == 3) {
        draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, 12 * g_ui_scale, 0x30FFFFFF, true);
    }
    draw_icon(cur_x, icon_y, icon_folder, ICON_FOLDER_WIDTH, ICON_FOLDER_HEIGHT);
    cur_x += icon_size + padding;
    
    /* 5. CMD / Settings */
    if (active_icon == 4) {
        draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, 12 * g_ui_scale, 0x30FFFFFF, true);
    }
    draw_icon(cur_x, icon_y, icon_cmd, ICON_CMD_WIDTH, ICON_CMD_HEIGHT);
    cur_x += icon_size + padding;
    
    /* 6. Power */
    if (active_icon == 5) draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, 16 * g_ui_scale, 0x30FFFFFF, true);
    draw_icon(cur_x, icon_y, icon_power, ICON_POWER_WIDTH, ICON_POWER_HEIGHT);
}

/* Draw System Tray                                                   */
/* ------------------------------------------------------------------ */
static void draw_system_tray(void) {
    int32_t tray_w = 240 * g_ui_scale;
    int32_t tray_h = 56 * g_ui_scale;
    int32_t tray_x = s_width - tray_w - (20 * g_ui_scale);
    int32_t tray_y = s_height - tray_h - (20 * g_ui_scale);
    
    /* Tray background (Match taskbar transparency) */
    apply_blur_rounded_rect(tray_x, tray_y, tray_w, tray_h, 6 * g_ui_scale, 28 * g_ui_scale);
    draw_rounded_rect(tray_x, tray_y, tray_w, tray_h, 28 * g_ui_scale, 0x602A2E33, true);
    
    /* Icons and time */
    font_draw_string_scaled(tray_x + (20 * g_ui_scale), tray_y + (12 * g_ui_scale), "[Wi-Fi]", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Read time again for tray */
    uint8_t m = get_rtc_register(0x02);
    uint8_t h = get_rtc_register(0x04);
    uint8_t regB = get_rtc_register(0x0B);
    if (!(regB & 0x04)) {
        m = bcd2bin(m);
        h = ((h & 0x0F) + (((h & 0x70) / 16) * 10)) | (h & 0x80);
    }
    if (!(regB & 0x02) && (h & 0x80)) {
        h = ((h & 0x7F) + 12) % 24;
    }
    char tray_time[6];
    tray_time[0] = '0' + (h / 10);
    tray_time[1] = '0' + (h % 10);
    tray_time[2] = ':';
    tray_time[3] = '0' + (m / 10);
    tray_time[4] = '0' + (m % 10);
    tray_time[5] = '\0';
    
    font_draw_string_scaled(tray_x + (140 * g_ui_scale), tray_y + (12 * g_ui_scale), tray_time, 0x00FFFFFF, 0x00000000, g_ui_scale);
}

/* ------------------------------------------------------------------ */
/* Draw Start Menu                                                    */
/* ------------------------------------------------------------------ */
static void draw_start_menu(void) {
    if (!s_start_menu_open) return;
    
    int32_t sm_width = 680 * g_ui_scale;
    int32_t sm_height = 600 * g_ui_scale;
    
    /* Calculate centered Start Menu */
    int32_t sm_x = (s_width - sm_width) / 2;
    int32_t sm_y = s_height - (90 * g_ui_scale) - sm_height; /* Floating above taskbar */
    
    /* Draw drop shadow behind menu */
    draw_shadow(sm_x, sm_y, sm_width, sm_height);
    
    /* Menu background (Dark mode Acrylic) with rounded corners */
    draw_rounded_rect(sm_x, sm_y, sm_width, sm_height, 16 * g_ui_scale, 0xF0202020, true);
    
    /* Search Bar */
    draw_rounded_rect(sm_x + (32 * g_ui_scale), sm_y + (32 * g_ui_scale), sm_width - (64 * g_ui_scale), 48 * g_ui_scale, 24 * g_ui_scale, 0xFF303030, false);
    font_draw_string_scaled(sm_x + (50 * g_ui_scale), sm_y + (40 * g_ui_scale), "Search apps...", 0x00A0A0A0, 0x00000000, g_ui_scale);
    
    /* Pinned Section Header */
    font_draw_string_scaled(sm_x + (40 * g_ui_scale), sm_y + (100 * g_ui_scale), "Pinned", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Grid of Pinned Apps */
    /* Centers: 100, 260, 420, 580 */
    /* 1. Terminal */
    draw_rounded_rect(sm_x + (76 * g_ui_scale), sm_y + (150 * g_ui_scale), 48 * g_ui_scale, 48 * g_ui_scale, 12 * g_ui_scale, 0xFF333333, false);
    font_draw_char_scaled(sm_x + (92 * g_ui_scale), sm_y + (158 * g_ui_scale), 'T', 0x004A90E2, 0x00000000, g_ui_scale);
    font_draw_string_scaled(sm_x + (36 * g_ui_scale), sm_y + (206 * g_ui_scale), "Terminal", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* 2. File Explorer */
    draw_rounded_rect(sm_x + (236 * g_ui_scale), sm_y + (150 * g_ui_scale), 48 * g_ui_scale, 48 * g_ui_scale, 12 * g_ui_scale, 0xFF333333, false);
    font_draw_char_scaled(sm_x + (252 * g_ui_scale), sm_y + (158 * g_ui_scale), 'F', 0x00F5A623, 0x00000000, g_ui_scale);
    font_draw_string_scaled(sm_x + (196 * g_ui_scale), sm_y + (206 * g_ui_scale), "Explorer", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* 3. System Info */
    draw_rounded_rect(sm_x + (396 * g_ui_scale), sm_y + (150 * g_ui_scale), 48 * g_ui_scale, 48 * g_ui_scale, 12 * g_ui_scale, 0xFF333333, false);
    font_draw_char_scaled(sm_x + (412 * g_ui_scale), sm_y + (158 * g_ui_scale), 'I', 0x0010B981, 0x00000000, g_ui_scale);
    font_draw_string_scaled(sm_x + (356 * g_ui_scale), sm_y + (206 * g_ui_scale), "Sys Info", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* 4. Settings */
    draw_rounded_rect(sm_x + (556 * g_ui_scale), sm_y + (150 * g_ui_scale), 48 * g_ui_scale, 48 * g_ui_scale, 12 * g_ui_scale, 0xFF333333, false);
    font_draw_char_scaled(sm_x + (572 * g_ui_scale), sm_y + (158 * g_ui_scale), 'S', 0x009B9B9B, 0x00000000, g_ui_scale);
    font_draw_string_scaled(sm_x + (516 * g_ui_scale), sm_y + (206 * g_ui_scale), "Settings", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Recommended Section Header */
    font_draw_string_scaled(sm_x + (40 * g_ui_scale), sm_y + (280 * g_ui_scale), "Recommended", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Recommended Item 1 */
    draw_rounded_rect(sm_x + (40 * g_ui_scale), sm_y + (330 * g_ui_scale), 32 * g_ui_scale, 32 * g_ui_scale, 8 * g_ui_scale, 0xFF4A90E2, false);
    font_draw_string_scaled(sm_x + (84 * g_ui_scale), sm_y + (330 * g_ui_scale), "genesi.iso", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Recommended Item 2 */
    draw_rounded_rect(sm_x + (360 * g_ui_scale), sm_y + (330 * g_ui_scale), 32 * g_ui_scale, 32 * g_ui_scale, 8 * g_ui_scale, 0xFFF5A623, false);
    font_draw_string_scaled(sm_x + (404 * g_ui_scale), sm_y + (330 * g_ui_scale), "kernel.c", 0x00FFFFFF, 0x00000000, g_ui_scale);

    /* Footer (User Profile & Power) */
    /* Draw footer background */
    draw_rounded_rect(sm_x, sm_y + sm_height - (64 * g_ui_scale), sm_width, 64 * g_ui_scale, 16 * g_ui_scale, 0xFF1A1A1A, false);
    /* Fix top corners of footer to be square (overwrite rounded corners from top) */
    fb_fillrect(sm_x, sm_y + sm_height - (64 * g_ui_scale), sm_width, 16 * g_ui_scale, 0x001A1A1A);
    
    /* User Avatar */
    draw_rounded_rect(sm_x + (40 * g_ui_scale), sm_y + sm_height - (48 * g_ui_scale), 32 * g_ui_scale, 32 * g_ui_scale, 16 * g_ui_scale, 0xFF4A90E2, false);
    font_draw_char_scaled(sm_x + (48 * g_ui_scale), sm_y + sm_height - (48 * g_ui_scale), 'U', 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Username */
    font_draw_string_scaled(sm_x + (84 * g_ui_scale), sm_y + sm_height - (48 * g_ui_scale), "Matheus", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Power Button */
    draw_rounded_rect(sm_x + sm_width - (72 * g_ui_scale), sm_y + sm_height - (48 * g_ui_scale), 32 * g_ui_scale, 32 * g_ui_scale, 8 * g_ui_scale, 0xFF2D2D2D, false);
    font_draw_char_scaled(sm_x + sm_width - (64 * g_ui_scale), sm_y + sm_height - (48 * g_ui_scale), 'O', 0x00FFFFFF, 0x00000000, g_ui_scale);
}

/* ------------------------------------------------------------------ */
/* Toggle start menu from keyboard                                    */
/* ------------------------------------------------------------------ */
void compositor_toggle_start_menu(void) {
    if (!fb_available()) return;
    s_start_menu_open = !s_start_menu_open;
    compositor_render();
}

/* ------------------------------------------------------------------ */
/* Toggle fullscreen for active window from keyboard (F11)            */
/* ------------------------------------------------------------------ */
void compositor_toggle_fullscreen(void) {
    if (!fb_available()) return;
    window_t *win = wm_get_top();
    if (!win) return;
    
    if (win->is_fullscreen) {
        win->is_fullscreen = false;
        win->x = win->saved_x;
        win->y = win->saved_y;
        wm_resize_window(win, win->saved_w, win->saved_h);
    } else {
        win->is_fullscreen = true;
        win->saved_x = win->x;
        win->saved_y = win->y;
        win->saved_w = win->width;
        win->saved_h = win->height;
        win->x = 0;
        win->y = 0; /* Cover everything including top bar */
        wm_resize_window(win, s_width, s_height);
    }
    compositor_render();
}

/* ------------------------------------------------------------------ */
/* Update GUI logic (mouse clicks, dragging, etc.)                    */
/* ------------------------------------------------------------------ */
void compositor_update(bool force) {
    if (!fb_available() || s_width == 0 || s_height == 0) return;

    static int32_t last_mx = -1, last_my = -1;
    static bool last_mdown = false;

    int32_t mx = mouse_x();
    int32_t my = mouse_y();
    bool    mdown = (mouse_buttons() & MOUSE_BTN_LEFT);

    /* Otimização drástica: Só re-renderiza a tela se o mouse mexer, clicar, ou houver force update */
    if (mx == last_mx && my == last_my && mdown == last_mdown && !force) {
        return;
    }
    
    /* Fast path for pure mouse movement (no drag, no resize, no click changes, no forced redraw) */
    if (mdown == last_mdown && !s_drag_win && !s_resizing_win && !force) {
        last_mx = mx;
        last_my = my;
        draw_cursor_direct(mx, my);
        return;
    }
    
    last_mx = mx;
    last_my = my;
    last_mdown = mdown;

    /* If mouse just went down */
    if (mdown && !s_mouse_was_down) {
        bool found = false;

        /* Check taskbar and start menu first */
        int32_t tb_width = 460 * g_ui_scale;
        int32_t tb_height = 64 * g_ui_scale;
        int32_t tb_x = (s_width - tb_width) / 2;
        int32_t tb_y = s_height - tb_height - (20 * g_ui_scale);
        
        if (my >= tb_y && my <= tb_y + tb_height && mx >= tb_x && mx <= tb_x + tb_width) {
            /* Click on taskbar */
            /* Check if clicked near the Start Menu icon (the first icon) */
            int32_t item_count = 6;
            int32_t icon_size = 32 * g_ui_scale;
            int32_t padding = (tb_width - (item_count * icon_size)) / (item_count + 1);
            int32_t start_x = tb_x + padding;
            
            if (mx >= start_x - padding/2 && mx <= start_x + icon_size + padding/2) {
                s_start_menu_open = !s_start_menu_open;
            } else {
                s_start_menu_open = false;
            }
            found = true;
        } else if (s_start_menu_open) {
            int32_t sm_width = 680 * g_ui_scale;
            int32_t sm_height = 600 * g_ui_scale;
            int32_t sm_x = (s_width - sm_width) / 2;
            int32_t sm_y = s_height - (90 * g_ui_scale) - sm_height;
            
            if (mx >= sm_x && mx <= sm_x + sm_width && my >= sm_y && my <= sm_y + sm_height) {
                /* Clicked inside start menu */
                extern void desktop_create_terminal(void);
                extern void desktop_create_explorer(void);
                extern void desktop_create_sysinfo(void);
                extern void desktop_create_settings(void);
                
                /* App 1: Terminal (Center 100) -> 76 to 124 */
                if (mx >= sm_x + (76 * g_ui_scale) && mx <= sm_x + (124 * g_ui_scale) && my >= sm_y + (150 * g_ui_scale) && my <= sm_y + (198 * g_ui_scale)) {
                    desktop_create_terminal();
                    s_start_menu_open = false;
                } 
                /* App 2: Explorer (Center 260) -> 236 to 284 */
                else if (mx >= sm_x + (236 * g_ui_scale) && mx <= sm_x + (284 * g_ui_scale) && my >= sm_y + (150 * g_ui_scale) && my <= sm_y + (198 * g_ui_scale)) {
                    desktop_create_explorer();
                    s_start_menu_open = false;
                }
                /* App 3: Sys Info (Center 420) -> 396 to 444 */
                else if (mx >= sm_x + (396 * g_ui_scale) && mx <= sm_x + (444 * g_ui_scale) && my >= sm_y + (150 * g_ui_scale) && my <= sm_y + (198 * g_ui_scale)) {
                    desktop_create_sysinfo();
                    s_start_menu_open = false;
                }
                /* App 4: Settings (Center 580) -> 556 to 604 */
                else if (mx >= sm_x + (556 * g_ui_scale) && mx <= sm_x + (604 * g_ui_scale) && my >= sm_y + (150 * g_ui_scale) && my <= sm_y + (198 * g_ui_scale)) {
                    desktop_create_settings();
                    s_start_menu_open = false;
                }
                /* Power Button */
                else if (mx >= sm_x + sm_width - (72 * g_ui_scale) && mx <= sm_x + sm_width - (40 * g_ui_scale) && my >= sm_y + sm_height - (48 * g_ui_scale) && my <= sm_y + sm_height - (16 * g_ui_scale)) {
                    extern void system_shutdown(void);
                    system_shutdown();
                }
                found = true;
            } else {
                /* Clicked somewhere else, close start menu */
                s_start_menu_open = false;
            }
        }

        if (!found) {
            /* Check windows from top to bottom */
            window_t *win = wm_get_top();
            while (win) {
            int32_t title_y = win->y - (56 * g_ui_scale);
            /* Check title bar click */
            if (mx >= win->x && mx <= win->x + (int32_t)win->width &&
                my >= title_y && my <= win->y) {
                
                /* Check if clicked the close button (x: 20, w: 14) */
                if (mx >= win->x + (20 * g_ui_scale) && mx <= win->x + (34 * g_ui_scale)) {
                    wm_destroy_window(win);
                    s_drag_win = NULL;
                    found = true;
                    break;
                }
                
                /* Minimize (x: 44, w: 14) */
                if (mx >= win->x + (44 * g_ui_scale) && mx <= win->x + (58 * g_ui_scale)) {
                    win->is_minimized = true;
                    s_drag_win = NULL;
                    found = true;
                    break;
                }
                
                /* Maximize (x: 68, w: 14) */
                if (mx >= win->x + (68 * g_ui_scale) && mx <= win->x + (82 * g_ui_scale)) {
                    if (win->is_maximized) {
                        win->is_maximized = false;
                        win->x = win->saved_x;
                        win->y = win->saved_y;
                        wm_resize_window(win, win->saved_w, win->saved_h);
                    } else {
                        win->is_maximized = true;
                        win->saved_x = win->x;
                        win->saved_y = win->y;
                        win->saved_w = win->width;
                        win->saved_h = win->height;
                        win->x = 0;
                        win->y = 56 * g_ui_scale; /* Title bar height */
                        wm_resize_window(win, s_width, s_height - (56 * g_ui_scale) - (84 * g_ui_scale));
                    }
                    wm_bring_to_front(win);
                    s_drag_win = NULL;
                    found = true;
                    break;
                }

                /* If double click on title bar? No double click yet. */
                wm_bring_to_front(win);
                if (!win->is_maximized) {
                    s_drag_win = win;
                    s_drag_off_x = mx - win->x;
                    s_drag_off_y = my - win->y;
                }
                found = true;
                break;
            }
            /* Check window body click */
            if (mx >= win->x && mx <= win->x + (int32_t)win->width &&
                my >= win->y && my <= win->y + (int32_t)win->height) {
                
                wm_bring_to_front(win);
                
                if (!win->is_maximized) {
                    int32_t edge = 8 * g_ui_scale;
                    bool on_right = (mx >= win->x + (int32_t)win->width - edge);
        bool on_bottom = (my >= win->y + (int32_t)win->height - edge);
                    
                    if (on_right || on_bottom) {
                        s_resizing_win = win;
                        if (on_right && on_bottom) s_resize_dir = 3;
                        else if (on_right) s_resize_dir = 1;
                        else if (on_bottom) s_resize_dir = 2;
                        
                        s_resize_start_w = win->width;
                        s_resize_start_h = win->height;
                        s_resize_start_mx = mx;
                        s_resize_start_my = my;
                        
                        found = true;
                        break;
                    }
                }
                
                /* Hack for Settings app: check if clicking wallpaper buttons */
                if (win->title[0] == 'S' && win->title[1] == 'e' && win->title[2] == 't') {
                    int32_t local_x = mx - win->x;
                    int32_t local_y = my - win->y;
                    
                    if (local_y >= 300 * g_ui_scale && local_y <= 350 * g_ui_scale) {
                        extern int g_current_wallpaper;
                        if (local_x >= 280 * g_ui_scale && local_x <= 460 * g_ui_scale) {
                            g_current_wallpaper = 0; /* Gradient */
                        } else if (local_x >= 480 * g_ui_scale && local_x <= 660 * g_ui_scale) {
                            g_current_wallpaper = 1; /* Picture */
                        }
                    }
                }
                
                /* App logic callback */
                if (win->on_mouse) {
                    win->on_mouse(win, mx - win->x, my - win->y, true);
                }
                
                found = true;
                break;
            }
            win = win->prev;
        }
        
        if (!found) {
            /* Clicked background/taskbar */
            s_drag_win = NULL;
        }
        }
    } 
    /* If mouse is held down and resizing a window */
    else if (mdown && s_resizing_win) {
        int32_t new_w = s_resize_start_w;
        int32_t new_h = s_resize_start_h;
        
        if (s_resize_dir & 1) new_w += (mx - s_resize_start_mx);
        if (s_resize_dir & 2) new_h += (my - s_resize_start_my);
        
        if (new_w < 100 * g_ui_scale) new_w = 100 * g_ui_scale;
        if (new_h < 50 * g_ui_scale) new_h = 50 * g_ui_scale;
        
        /* Just update the state variables to draw the outline in render() */
        /* Wait, we can't store the outline size in s_resizing_win without affecting draw_window.
           We'll just use s_resize_start_w/h + mouse diff in the render loop. */
    }
    /* If mouse is held down and dragging a window */
    else if (mdown && s_drag_win) {
        s_drag_win->x = mx - s_drag_off_x;
        s_drag_win->y = my - s_drag_off_y;
    } 
    /* If mouse was released */
    else if (!mdown) {
        if (s_resizing_win) {
            int32_t new_w = s_resize_start_w;
            int32_t new_h = s_resize_start_h;
            
            if (s_resize_dir & 1) new_w += (mx - s_resize_start_mx);
            if (s_resize_dir & 2) new_h += (my - s_resize_start_my);
            
            if (new_w < 100 * g_ui_scale) new_w = 100 * g_ui_scale;
            if (new_h < 50 * g_ui_scale) new_h = 50 * g_ui_scale;
            
            wm_resize_window(s_resizing_win, new_w, new_h);
            s_resizing_win = NULL;
        }
        s_drag_win = NULL;
    }

    s_mouse_was_down = mdown;

    /* Render everything */
    compositor_render();
}

void compositor_render(void) {
    if (!fb_available() || s_width == 0 || s_height == 0) return;
    
    window_t *top = wm_get_top();
    bool is_fs = (top && top->is_fullscreen && !top->is_minimized);

    /* 1. Clear background (Wallpaper) */
    if (!is_fs) {
        draw_wallpaper();
    }

    /* 2. Desktop Widgets */
    if (!is_fs) {
        draw_desktop_widgets();
    }

    /* 3. Draw windows back-to-front */
    window_t *win = wm_get_bottom();
    while (win) {
        draw_window(win);
        win = win->next;
    }

    if (!is_fs) {
        /* 4. Draw Taskbar */
        draw_taskbar();

        /* 5. System Tray */
        draw_system_tray();

        /* 6. Draw Start Menu */
        draw_start_menu();
    }
    
    /* Draw resize outline */
    if (s_resizing_win) {
        extern int32_t mouse_x(void);
        extern int32_t mouse_y(void);
        int32_t mx = mouse_x();
        int32_t my = mouse_y();
        
        int32_t new_w = s_resize_start_w;
        int32_t new_h = s_resize_start_h;
        
        if (s_resize_dir & 1) new_w += (mx - s_resize_start_mx);
        if (s_resize_dir & 2) new_h += (my - s_resize_start_my);
        
        if (new_w < 100 * g_ui_scale) new_w = 100 * g_ui_scale;
        if (new_h < 50 * g_ui_scale) new_h = 50 * g_ui_scale;
        
        int32_t title_h = 56 * g_ui_scale;
        
        /* Draw hollow outline */
        int32_t t = 4 * g_ui_scale; /* thickness */
        draw_rounded_rect(s_resizing_win->x, s_resizing_win->y - title_h, new_w, t, 0, 0x80FFFFFF, true); /* top */
        draw_rounded_rect(s_resizing_win->x, s_resizing_win->y - title_h + new_h + title_h - t, new_w, t, 0, 0x80FFFFFF, true); /* bottom */
        draw_rounded_rect(s_resizing_win->x, s_resizing_win->y - title_h, t, new_h + title_h, 0, 0x80FFFFFF, true); /* left */
        draw_rounded_rect(s_resizing_win->x + new_w - t, s_resizing_win->y - title_h, t, new_h + title_h, 0, 0x80FFFFFF, true); /* right */
    }

    /* 6. Flip backbuffer to screen */
    fb_flip();
    
    /* Force refresh cursor to screen (since backbuffer changed underneath it) */
    s_old_cx = -1; /* Invalidate old cursor rect since we just flipped */
    draw_cursor_direct(mouse_x(), mouse_y());
}
