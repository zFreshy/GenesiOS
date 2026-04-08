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

static window_t *s_drag_win   = NULL;
static int32_t   s_drag_off_x = 0;
static int32_t   s_drag_off_y = 0;
static bool      s_mouse_was_down = false;
static bool      s_start_menu_open = false;

void compositor_render(void);

/* ------------------------------------------------------------------ */
/* Initialize compositor                                              */
/* ------------------------------------------------------------------ */
void compositor_init(void) {
    if (!fb_available()) return;
    s_width  = fb_width();
    s_height = fb_height();
    
    /* Sempre usar escala 1 para evitar lentidão e tamanhos gigantes */
    g_ui_scale = 1;
}

/* ------------------------------------------------------------------ */
/* Draw Wallpaper (Aladin OS Style Gradient)                          */
/* ------------------------------------------------------------------ */
static void draw_wallpaper(void) {
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (!bb) return;

    /* A fast 1D vertical gradient to completely eliminate lag */
    /* Light purple to soft blue */
    for (uint32_t y = 0; y < s_height; y++) {
        uint32_t t = (y * 255) / s_height;
        uint32_t r = 0xE6 + (t * (0x90 - 0xE6)) / 255;
        uint32_t g = 0xE6 + (t * (0xA0 - 0xE6)) / 255;
        uint32_t b = 0xFA + (t * (0xE0 - 0xFA)) / 255;
        
        uint32_t color = (r << 16) | (g << 8) | b;
        
        for (uint32_t x = 0; x < s_width; x++) {
            bb[y * s_width + x] = color;
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
        for (int32_t cx = x; cx < x + w; cx++) {
            if (cx < 0 || cx >= (int32_t)s_width || cy < 0 || cy >= (int32_t)s_height) continue;

            /* Distância até a quina mais próxima para desenhar o círculo */
            int32_t dx = 0, dy = 0;
            if (cx < x + r) dx = (x + r - 1) - cx;
            else if (cx >= x + w - r) dx = cx - (x + w - r);
            
            if (cy < y + r) dy = (y + r - 1) - cy;
            else if (cy >= y + h - r) dy = cy - (y + h - r);
            
            uint32_t dist_sq = dx*dx + dy*dy;
            uint32_t rr = r*r;
            uint32_t alpha = base_alpha;
            
            if (dist_sq >= rr) {
                continue; /* Fora do círculo */
            } else if (dist_sq > (r - 2)*(r - 2)) {
                /* Borda do círculo: aplica Anti-Aliasing (suavização) */
                uint32_t dist = fast_sqrt(dist_sq);
                if (dist >= r) continue;
                /* Calcula opacidade baseada em quão perto do sub-pixel está (0 a 255) */
                uint32_t edge_alpha = 255 - ((dist - (r - 2)) * 255 / 2);
                alpha = (alpha * edge_alpha) / 255;
            }

            if (blend && alpha < 255) {
                uint32_t bg = bb[cy * s_width + cx];
                uint32_t bgr = (bg >> 16) & 0xFF;
                uint32_t bgg = (bg >> 8) & 0xFF;
                uint32_t bgb = bg & 0xFF;
                uint32_t nr = (cr * alpha + bgr * (255 - alpha)) / 255;
                uint32_t ng = (cg * alpha + bgg * (255 - alpha)) / 255;
                uint32_t nb = (cb * alpha + bgb * (255 - alpha)) / 255;
                bb[cy * s_width + cx] = (nr << 16) | (ng << 8) | nb;
            } else {
                bb[cy * s_width + cx] = color & 0xFFFFFF;
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
            
            uint32_t pitch_words = s_width;
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

    int32_t title_h = 36 * g_ui_scale;
    int32_t win_radius = 16 * g_ui_scale; /* Rounded corners */

    /* Draw window drop shadow */
    if (win->x > 0 && win->y > title_h) {
        draw_shadow(win->x, win->y - title_h, win->width, win->height + title_h);
    }

    /* Draw window title bar (semi-transparent light blur effect) */
    if (win->y >= title_h) {
        int32_t title_y = win->y - title_h;
        
        /* Light theme: Frosty white title bar */
        uint32_t tb_color = (win == wm_get_top()) ? 0xE0F0F5FA : 0xD0E8EDF2;

        /* Draw title bar background (rounded top) */
        draw_rounded_rect(win->x, title_y, win->width, title_h + win_radius, win_radius, tb_color, true);

        /* Title text */
        uint32_t text_color = 0x004A5568; /* Dark greyish blue */
        font_draw_string_scaled(win->x + (20 * g_ui_scale), title_y + (12 * g_ui_scale), win->title, text_color, 0x00000000, g_ui_scale);

        /* Control buttons (MacOS/Aladin style traffic lights on the right) */
        int32_t btn_y = title_y + (title_h - (12 * g_ui_scale)) / 2;
        int32_t btn_s = 14 * g_ui_scale;
        int32_t btn_r = 7 * g_ui_scale;
        
        /* Minimize (Yellow) */
        draw_rounded_rect(win->x + win->width - (70 * g_ui_scale), btn_y, btn_s, btn_s, btn_r, 0xFFF5A623, false);
        
        /* Maximize (Green) */
        draw_rounded_rect(win->x + win->width - (45 * g_ui_scale), btn_y, btn_s, btn_s, btn_r, 0xFF10B981, false);

        /* Close (Red) */
        draw_rounded_rect(win->x + win->width - (20 * g_ui_scale), btn_y, btn_s, btn_s, btn_r, 0xFFEF4444, false);
    }

    /* Draw window content buffer (rounded bottom) */
    if (win->buffer) {
        /* We can't easily mask the buffer with rounded corners without a proper blit function, 
           so we just blit it directly. The desktop apps should ideally draw their own backgrounds rounded.
           For now, a standard blit is fine. */
        fb_blit(win->x, win->y, win->width, win->height, win->buffer);
    }
}

/* ------------------------------------------------------------------ */
/* Draw an arrow cursor                                               */
/* ------------------------------------------------------------------ */
static void draw_cursor(void) {
    int32_t cx = mouse_x();
    int32_t cy = mouse_y();

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
                        if (c == 'X') {
                            fb_putpixel(cx + x * g_ui_scale + sx, cy + y * g_ui_scale + sy, 0x00000000);
                        } else if (c == '.') {
                            fb_putpixel(cx + x * g_ui_scale + sx, cy + y * g_ui_scale + sy, 0x00FFFFFF);
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
static void draw_desktop_widgets(void) {
    /* Logo / System Name at Top Right */
    uint32_t brand_color = 0x00FFFFFF; /* White */
    font_draw_string_scaled(s_width - (120 * g_ui_scale), 20 * g_ui_scale, "GENESI OS", brand_color, 0x00000000, g_ui_scale);
    
    /* Huge Clock at Top Center */
    uint32_t clock_x = (s_width - (5 * 16 * g_ui_scale)) / 2; /* 5 chars */
    font_draw_string_scaled(clock_x + 2 * g_ui_scale, 42 * g_ui_scale, "08:20", 0x005555AA, 0x00000000, g_ui_scale); /* Drop shadow */
    font_draw_string_scaled(clock_x, 40 * g_ui_scale, "08:20", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Date below clock */
    uint32_t date_x = (s_width - (11 * 16 * g_ui_scale)) / 2; /* 11 chars */
    font_draw_string_scaled(date_x, 70 * g_ui_scale, "Fri, Aug 28", 0x00D0D0E0, 0x00000000, g_ui_scale);
}

/* ------------------------------------------------------------------ */
/* Draw taskbar (Floating Dock)                                       */
/* ------------------------------------------------------------------ */
static void draw_taskbar(void) {
    int32_t tb_height = 56 * g_ui_scale;
    int32_t tb_width = 400 * g_ui_scale;
    int32_t tb_x = (s_width - tb_width) / 2;
    int32_t tb_y = s_height - tb_height - (20 * g_ui_scale); /* Floating */
    
    /* Taskbar background (Light frosted glass) */
    draw_rounded_rect(tb_x, tb_y, tb_width, tb_height, 28 * g_ui_scale, 0xD0FFFFFF, true);
    
    /* Draw Start button background */
    int32_t start_size = 40 * g_ui_scale;
    int32_t start_x = tb_x + (12 * g_ui_scale);
    int32_t start_y = tb_y + (8 * g_ui_scale);
    draw_rounded_rect(start_x, start_y, start_size, start_size, 20 * g_ui_scale, 0xFF4A90E2, true);
    
    /* Draw Start button icon (SVG -> PNG -> C Array) with Bilinear Interpolation */
    extern uint32_t *fb_get_backbuffer(void);
    uint32_t *bb = fb_get_backbuffer();
    if (bb) {
        int32_t scaled_iw = ICON_START_WIDTH * g_ui_scale;
        int32_t scaled_ih = ICON_START_HEIGHT * g_ui_scale;
        int32_t icon_x = start_x + (start_size - scaled_iw) / 2;
        int32_t icon_y = start_y + (start_size - scaled_ih) / 2;
        
        for (int32_t iy = 0; iy < scaled_ih; iy++) {
            for (int32_t ix = 0; ix < scaled_iw; ix++) {
                if (icon_x + ix < 0 || icon_x + ix >= (int32_t)s_width ||
                    icon_y + iy < 0 || icon_y + iy >= (int32_t)s_height) {
                    continue;
                }
                
                if (g_ui_scale == 1) {
                    uint32_t color = icon_start[iy * ICON_START_WIDTH + ix];
                    uint32_t alpha = (color >> 24) & 0xFF;
                    if (alpha == 255) {
                        bb[(icon_y + iy) * s_width + (icon_x + ix)] = color & 0xFFFFFF;
                    } else if (alpha > 0) {
                        uint32_t bg = bb[(icon_y + iy) * s_width + (icon_x + ix)];
                        uint32_t bgr = (bg >> 16) & 0xFF;
                        uint32_t bgg = (bg >> 8) & 0xFF;
                        uint32_t bgb = bg & 0xFF;
                        uint32_t fr = (color >> 16) & 0xFF;
                        uint32_t fg = (color >> 8) & 0xFF;
                        uint32_t fb = color & 0xFF;
                        uint32_t r = (fr * alpha + bgr * (255 - alpha)) / 255;
                        uint32_t g = (fg * alpha + bgg * (255 - alpha)) / 255;
                        uint32_t b = (fb * alpha + bgb * (255 - alpha)) / 255;
                        bb[(icon_y + iy) * s_width + (icon_x + ix)] = (r << 16) | (g << 8) | b;
                    }
                    continue;
                }
                
                uint32_t src_x = (ix * 256) / g_ui_scale;
                uint32_t src_y = (iy * 256) / g_ui_scale;
                uint32_t x0 = src_x >> 8;
                uint32_t y0 = src_y >> 8;
                uint32_t x1 = x0 + 1;
                uint32_t y1 = y0 + 1;
                if (x1 >= ICON_START_WIDTH) x1 = ICON_START_WIDTH - 1;
                if (y1 >= ICON_START_HEIGHT) y1 = ICON_START_HEIGHT - 1;
                
                uint32_t fx = src_x & 0xFF;
                uint32_t fy = src_y & 0xFF;
                
                uint32_t c00 = icon_start[y0 * ICON_START_WIDTH + x0];
                uint32_t c01 = icon_start[y0 * ICON_START_WIDTH + x1];
                uint32_t c10 = icon_start[y1 * ICON_START_WIDTH + x0];
                uint32_t c11 = icon_start[y1 * ICON_START_WIDTH + x1];
                
                /* Bilinear blend of alpha */
                uint32_t a00 = (c00 >> 24) & 0xFF, a01 = (c01 >> 24) & 0xFF;
                uint32_t a10 = (c10 >> 24) & 0xFF, a11 = (c11 >> 24) & 0xFF;
                uint32_t top_a = (a00 * (256 - fx) + a01 * fx) >> 8;
                uint32_t bot_a = (a10 * (256 - fx) + a11 * fx) >> 8;
                uint32_t alpha = (top_a * (256 - fy) + bot_a * fy) >> 8;
                
                if (alpha > 0) {
                    /* Bilinear blend of RGB */
                    uint32_t r00 = (c00 >> 16) & 0xFF, r01 = (c01 >> 16) & 0xFF;
                    uint32_t r10 = (c10 >> 16) & 0xFF, r11 = (c11 >> 16) & 0xFF;
                    uint32_t top_r = (r00 * (256 - fx) + r01 * fx) >> 8;
                    uint32_t bot_r = (r10 * (256 - fx) + r11 * fx) >> 8;
                    uint32_t fr = (top_r * (256 - fy) + bot_r * fy) >> 8;
                    
                    uint32_t g00 = (c00 >> 8) & 0xFF, g01 = (c01 >> 8) & 0xFF;
                    uint32_t g10 = (c10 >> 8) & 0xFF, g11 = (c11 >> 8) & 0xFF;
                    uint32_t top_g = (g00 * (256 - fx) + g01 * fx) >> 8;
                    uint32_t bot_g = (g10 * (256 - fx) + g11 * fx) >> 8;
                    uint32_t fg = (top_g * (256 - fy) + bot_g * fy) >> 8;
                    
                    uint32_t b00 = c00 & 0xFF, b01 = c01 & 0xFF;
                    uint32_t b10 = c10 & 0xFF, b11 = c11 & 0xFF;
                    uint32_t top_b = (b00 * (256 - fx) + b01 * fx) >> 8;
                    uint32_t bot_b = (b10 * (256 - fx) + b11 * fx) >> 8;
                    uint32_t fb = (top_b * (256 - fy) + bot_b * fy) >> 8;
                    
                    if (alpha == 255) {
                        bb[(icon_y + iy) * s_width + (icon_x + ix)] = (fr << 16) | (fg << 8) | fb;
                    } else {
                        uint32_t bg = bb[(icon_y + iy) * s_width + (icon_x + ix)];
                        uint32_t bgr = (bg >> 16) & 0xFF;
                        uint32_t bgg = (bg >> 8) & 0xFF;
                        uint32_t bgb = bg & 0xFF;
                        
                        uint32_t r = (fr * alpha + bgr * (255 - alpha)) / 255;
                        uint32_t g = (fg * alpha + bgg * (255 - alpha)) / 255;
                        uint32_t b = (fb * alpha + bgb * (255 - alpha)) / 255;
                        
                        bb[(icon_y + iy) * s_width + (icon_x + ix)] = (r << 16) | (g << 8) | b;
                    }
                }
            }
        }
    }

    /* Draw buttons for windows */
    int num_windows = 0;
    window_t *w = wm_get_bottom();
    while(w) { num_windows++; w = w->next; }
    
    window_t *win = wm_get_bottom();
    int32_t btn_x = start_x + (60 * g_ui_scale);
    int32_t btn_size = 32 * g_ui_scale;
    while (win) {
        uint32_t btn_color = (win == wm_get_top()) ? 0xFFFFFFFF : 0x80FFFFFF;
        draw_rounded_rect(btn_x, tb_y + (12 * g_ui_scale), btn_size, btn_size, 8 * g_ui_scale, btn_color, true);
        /* Draw little icon indicator */
        font_draw_char_scaled(btn_x + (8 * g_ui_scale), tb_y + (0 * g_ui_scale), win->title[0], 0x004A90E2, 0x00000000, g_ui_scale);
        btn_x += (44 * g_ui_scale);
        win = win->next;
    }
}

/* ------------------------------------------------------------------ */
/* Draw System Tray                                                   */
/* ------------------------------------------------------------------ */
static void draw_system_tray(void) {
    int32_t tray_w = 200 * g_ui_scale;
    int32_t tray_h = 44 * g_ui_scale;
    int32_t tray_x = s_width - tray_w - (20 * g_ui_scale);
    int32_t tray_y = s_height - tray_h - (20 * g_ui_scale);
    
    /* Tray background */
    draw_rounded_rect(tray_x, tray_y, tray_w, tray_h, 22 * g_ui_scale, 0xD0FFFFFF, true);
    
    /* Icons and time */
    font_draw_string_scaled(tray_x + (20 * g_ui_scale), tray_y + (10 * g_ui_scale), "[Wi-Fi]", 0x004A5568, 0x00000000, g_ui_scale);
    font_draw_string_scaled(tray_x + (110 * g_ui_scale), tray_y + (10 * g_ui_scale), "08:20", 0x001A202C, 0x00000000, g_ui_scale);
}

/* ------------------------------------------------------------------ */
/* Draw Start Menu                                                    */
/* ------------------------------------------------------------------ */
static void draw_start_menu(void) {
    if (!s_start_menu_open) return;
    
    int32_t sm_width = 240 * g_ui_scale;
    int32_t sm_height = 320 * g_ui_scale;
    
    /* Calculate centered Start Menu */
    int32_t sm_x = (s_width - sm_width) / 2;
    int32_t sm_y = s_height - (40 * g_ui_scale) - sm_height - (12 * g_ui_scale); /* Floating above taskbar */
    
    /* Draw drop shadow behind menu */
    draw_shadow(sm_x, sm_y, sm_width, sm_height);
    
    /* Menu border */
    fb_fillrect(sm_x - 1, sm_y - 1, sm_width + 2, sm_height + 2, 0x00333333);
    /* Menu background (Dark mode Acrylic) */
    fb_fillrect(sm_x, sm_y, sm_width, sm_height, 0x00202020);
    
    font_draw_string_scaled(sm_x + (20 * g_ui_scale), sm_y + (20 * g_ui_scale), "Pinned Apps", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Menu items */
    /* Item 1: Terminal */
    fb_fillrect(sm_x + (16 * g_ui_scale), sm_y + (48 * g_ui_scale), sm_width - (32 * g_ui_scale), 32 * g_ui_scale, 0x002D2D2D);
    font_draw_string_scaled(sm_x + (32 * g_ui_scale), sm_y + (60 * g_ui_scale), "Terminal", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Item 2: File Explorer */
    fb_fillrect(sm_x + (16 * g_ui_scale), sm_y + (88 * g_ui_scale), sm_width - (32 * g_ui_scale), 32 * g_ui_scale, 0x002D2D2D);
    font_draw_string_scaled(sm_x + (32 * g_ui_scale), sm_y + (100 * g_ui_scale), "File Explorer", 0x00FFFFFF, 0x00000000, g_ui_scale);
    
    /* Item 3: System Info */
    fb_fillrect(sm_x + (16 * g_ui_scale), sm_y + (128 * g_ui_scale), sm_width - (32 * g_ui_scale), 32 * g_ui_scale, 0x002D2D2D);
    font_draw_string_scaled(sm_x + (32 * g_ui_scale), sm_y + (140 * g_ui_scale), "System Info", 0x00FFFFFF, 0x00000000, g_ui_scale);

    /* Separator line */
    fb_fillrect(sm_x + (16 * g_ui_scale), sm_y + sm_height - (60 * g_ui_scale), sm_width - (32 * g_ui_scale), 1 * g_ui_scale, 0x00333333);

    /* Item 4: Shut Down */
    fb_fillrect(sm_x + (16 * g_ui_scale), sm_y + sm_height - (48 * g_ui_scale), sm_width - (32 * g_ui_scale), 32 * g_ui_scale, 0x00C42B1C); /* Red button */
    font_draw_string_scaled(sm_x + (32 * g_ui_scale), sm_y + sm_height - (36 * g_ui_scale), "Shut Down", 0x00FFFFFF, 0x00000000, g_ui_scale);
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
/* Update GUI logic (mouse clicks, dragging, etc.)                    */
/* ------------------------------------------------------------------ */
void compositor_update(void) {
    if (!fb_available() || s_width == 0 || s_height == 0) return;

    static int32_t last_mx = -1, last_my = -1;
    static bool last_mdown = false;

    int32_t mx = mouse_x();
    int32_t my = mouse_y();
    bool    mdown = (mouse_buttons() & MOUSE_BTN_LEFT);

    /* Otimização drástica: Só re-renderiza a tela inteira se o mouse mexer ou clicar */
    if (mx == last_mx && my == last_my && mdown == last_mdown) {
        return;
    }
    
    last_mx = mx;
    last_my = my;
    last_mdown = mdown;

    /* If mouse just went down */
    if (mdown && !s_mouse_was_down) {
        bool found = false;

        /* Check taskbar and start menu first */
        if (my >= (int32_t)s_height - (40 * g_ui_scale)) {
            /* Click on taskbar */
            int num_windows = 0;
            window_t *w = wm_get_bottom();
            while(w) { num_windows++; w = w->next; }
            
            /* Recalculate exact total width of taskbar items just like draw_taskbar */
            int32_t btn_width = 40 * g_ui_scale;
            int32_t start_btn_width = 48 * g_ui_scale;
            int32_t total_width = start_btn_width + (num_windows * (btn_width + (8 * g_ui_scale)));
            int32_t start_x = (s_width - total_width) / 2;
            
            if (mx >= start_x && mx <= start_x + start_btn_width) {
                s_start_menu_open = !s_start_menu_open;
            } else {
                s_start_menu_open = false;
            }
            found = true;
        } else if (s_start_menu_open && mx >= (int32_t)(s_width - (240 * g_ui_scale))/2 && mx <= (int32_t)(s_width + (240 * g_ui_scale))/2 && my >= (int32_t)s_height - (40 * g_ui_scale) - (320 * g_ui_scale) - (12 * g_ui_scale) && my < (int32_t)s_height - (40 * g_ui_scale)) {
            /* Clicked inside start menu */
            int32_t sm_y = (int32_t)s_height - (40 * g_ui_scale) - (320 * g_ui_scale) - (12 * g_ui_scale);
            extern void desktop_create_terminal(void);
            extern void desktop_create_explorer(void);
            extern void desktop_create_sysinfo(void);
            
            if (my >= sm_y + (48 * g_ui_scale) && my < sm_y + (80 * g_ui_scale)) {
                desktop_create_terminal();
            } else if (my >= sm_y + (88 * g_ui_scale) && my < sm_y + (120 * g_ui_scale)) {
                desktop_create_explorer();
            } else if (my >= sm_y + (128 * g_ui_scale) && my < sm_y + (160 * g_ui_scale)) {
                desktop_create_sysinfo();
            } else if (my >= sm_y + (320 * g_ui_scale) - (48 * g_ui_scale) && my < sm_y + (320 * g_ui_scale) - (16 * g_ui_scale)) {
                extern void system_shutdown(void);
                system_shutdown();
            }
            s_start_menu_open = false;
            found = true;
        } else {
            /* Clicked somewhere else, close start menu */
            s_start_menu_open = false;
        }

        if (!found) {
            /* Check windows from top to bottom */
            window_t *win = wm_get_top();
            while (win) {
            int32_t title_y = win->y - (24 * g_ui_scale);
            /* Check title bar click */
            if (mx >= win->x && mx <= win->x + (int32_t)win->width &&
                my >= title_y && my <= win->y) {
                
                /* Check if clicked the close button (width 32) */
                if (mx >= win->x + (int32_t)win->width - (32 * g_ui_scale)) {
                    wm_destroy_window(win);
                    s_drag_win = NULL;
                    found = true;
                    break;
                }

                wm_bring_to_front(win);
                s_drag_win = win;
                s_drag_off_x = mx - win->x;
                s_drag_off_y = my - win->y;
                found = true;
                break;
            }
            /* Check window body click */
            if (mx >= win->x && mx <= win->x + (int32_t)win->width &&
                my >= win->y && my <= win->y + (int32_t)win->height) {
                
                wm_bring_to_front(win);
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
    /* If mouse is held down and dragging a window */
    else if (mdown && s_drag_win) {
        s_drag_win->x = mx - s_drag_off_x;
        s_drag_win->y = my - s_drag_off_y;
    } 
    /* If mouse was released */
    else if (!mdown) {
        s_drag_win = NULL;
    }

    s_mouse_was_down = mdown;

    /* Render everything */
    compositor_render();
}

/* ------------------------------------------------------------------ */
/* Render all layers to screen                                        */
/* ------------------------------------------------------------------ */
void compositor_render(void) {
    if (!fb_available() || s_width == 0 || s_height == 0) return;

    /* 1. Clear background (Wallpaper) */
    draw_wallpaper();

    /* 2. Desktop Widgets */
    draw_desktop_widgets();

    /* 3. Draw windows back-to-front */
    window_t *win = wm_get_bottom();
    while (win) {
        draw_window(win);
        win = win->next;
    }

    /* 4. Draw Taskbar */
    draw_taskbar();

    /* 5. System Tray */
    draw_system_tray();

    /* 6. Draw Start Menu */
    draw_start_menu();

    /* 5. Draw mouse cursor overlay */
    draw_cursor();

    /* 6. Flip backbuffer to screen */
    fb_flip();
}
