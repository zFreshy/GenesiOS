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

static uint32_t  s_width      = 0;
static uint32_t  s_height     = 0;

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
}

/* ------------------------------------------------------------------ */
/* Draw a single window                                               */
/* ------------------------------------------------------------------ */
static void draw_window(window_t *win) {
    if (!win) return;

    int32_t title_h = 24;

    /* Draw window border (1px) and shadow effect */
    uint32_t border_color = (win == wm_get_top()) ? 0x00555555 : 0x00333333;
    if (win->x > 0 && win->y > title_h) {
        fb_fillrect(win->x - 1, win->y - title_h - 1, win->width + 2, win->height + title_h + 2, border_color);
    }

    /* Draw window title bar */
    if (win->y >= title_h) {
        int32_t title_y = win->y - title_h;
        
        /* Check if it's the top window for color */
        uint32_t tb_color = (win == wm_get_top()) ? 0x002D2D2D : 0x001A1A1A;

        /* Draw title bar background */
        fb_fillrect(win->x, title_y, win->width, title_h, tb_color);

        /* Draw close button (clean dark red box) */
        fb_fillrect(win->x + win->width - 32, title_y, 32, title_h, 0x00C42B1C);
        font_draw_char(win->x + win->width - 16 - 4, title_y + 8, 'X', 0x00FFFFFF, 0x00000000);

        /* Draw title text centered vertically */
        font_draw_string(win->x + 12, title_y + 8, win->title, 0x00FFFFFF, 0x00000000);
    }

    /* Draw window content buffer */
    if (win->buffer) {
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
            if (c == 'X') {
                fb_putpixel(cx + x, cy + y, 0x00000000); /* Black border */
            } else if (c == '.') {
                fb_putpixel(cx + x, cy + y, 0x00FFFFFF); /* White fill */
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw taskbar                                                       */
/* ------------------------------------------------------------------ */
static void draw_taskbar(void) {
    int32_t tb_height = 40;
    int32_t tb_y = s_height - tb_height;
    
    /* Taskbar background (Acrylic/Dark mode) */
    fb_fillrect(0, tb_y, s_width, tb_height, 0x00141414);
    
    /* Top border line for taskbar */
    fb_fillrect(0, tb_y, s_width, 1, 0x002A2A2A);

    /* Calculate centered icons width */
    int num_windows = 0;
    window_t *w = wm_get_bottom();
    while(w) { num_windows++; w = w->next; }
    
    int32_t btn_width = 40;
    int32_t start_btn_width = 48;
    int32_t total_width = start_btn_width + (num_windows * (btn_width + 8));
    int32_t start_x = (s_width - total_width) / 2;

    /* Start button (Windows 11 style rounded-like icon) */
    /* Draw 4 blue squares */
    uint32_t start_color = s_start_menu_open ? 0x002D2D2D : 0x001A1A1A;
    fb_fillrect(start_x, tb_y + 4, start_btn_width, 32, start_color);
    
    /* Logo squares */
    uint32_t win_blue = 0x000078D7;
    fb_fillrect(start_x + 14, tb_y + 10, 9, 9, win_blue);
    fb_fillrect(start_x + 25, tb_y + 10, 9, 9, win_blue);
    fb_fillrect(start_x + 14, tb_y + 21, 9, 9, win_blue);
    fb_fillrect(start_x + 25, tb_y + 21, 9, 9, win_blue);

    /* Draw buttons for windows */
    window_t *win = wm_get_bottom();
    int32_t btn_x = start_x + start_btn_width + 8;
    while (win) {
        uint32_t btn_color = (win == wm_get_top()) ? 0x002D2D2D : 0x001A1A1A;
        fb_fillrect(btn_x, tb_y + 4, btn_width, 32, btn_color);
        /* Draw first letter of window title as icon */
        font_draw_char(btn_x + 16, tb_y + 16, win->title[0], 0x00FFFFFF, 0x00000000);
        
        /* Small dot under active window */
        if (win == wm_get_top()) {
            fb_fillrect(btn_x + 16, tb_y + 32, 8, 2, 0x000078D7);
        }
        
        btn_x += btn_width + 8;
        win = win->next;
    }

    /* Clock/Status area */
    font_draw_string(s_width - 80, tb_y + 16, "Genesi OS", 0x00888888, 0x00000000);
}

/* ------------------------------------------------------------------ */
/* Draw Start Menu                                                    */
/* ------------------------------------------------------------------ */
static void draw_start_menu(void) {
    if (!s_start_menu_open) return;
    
    int32_t sm_width = 240;
    int32_t sm_height = 320;
    
    /* Calculate centered Start Menu */
    int32_t sm_x = (s_width - sm_width) / 2;
    int32_t sm_y = s_height - 40 - sm_height - 12; /* Floating above taskbar */
    
    /* Menu border */
    fb_fillrect(sm_x - 1, sm_y - 1, sm_width + 2, sm_height + 2, 0x00333333);
    /* Menu background (Dark mode Acrylic) */
    fb_fillrect(sm_x, sm_y, sm_width, sm_height, 0x00202020);
    
    font_draw_string(sm_x + 20, sm_y + 20, "Pinned Apps", 0x00FFFFFF, 0x00000000);
    
    /* Menu items */
    /* Item 1: Terminal */
    fb_fillrect(sm_x + 16, sm_y + 48, sm_width - 32, 32, 0x002D2D2D);
    font_draw_string(sm_x + 32, sm_y + 60, "Terminal", 0x00FFFFFF, 0x00000000);
    
    /* Item 2: File Explorer */
    fb_fillrect(sm_x + 16, sm_y + 88, sm_width - 32, 32, 0x002D2D2D);
    font_draw_string(sm_x + 32, sm_y + 100, "File Explorer", 0x00FFFFFF, 0x00000000);
    
    /* Item 3: System Info */
    fb_fillrect(sm_x + 16, sm_y + 128, sm_width - 32, 32, 0x002D2D2D);
    font_draw_string(sm_x + 32, sm_y + 140, "System Info", 0x00FFFFFF, 0x00000000);

    /* Separator line */
    fb_fillrect(sm_x + 16, sm_y + sm_height - 60, sm_width - 32, 1, 0x00333333);

    /* Item 4: Shut Down */
    fb_fillrect(sm_x + 16, sm_y + sm_height - 48, sm_width - 32, 32, 0x00C42B1C); /* Red button */
    font_draw_string(sm_x + 32, sm_y + sm_height - 36, "Shut Down", 0x00FFFFFF, 0x00000000);
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
    if (!fb_available()) return;

    int32_t mx = mouse_x();
    int32_t my = mouse_y();
    bool    mdown = (mouse_buttons() & MOUSE_BTN_LEFT);

    /* If mouse just went down */
    if (mdown && !s_mouse_was_down) {
        bool found = false;

        /* Check taskbar and start menu first */
        if (my >= (int32_t)s_height - 40) {
            /* Click on taskbar */
            int num_windows = 0;
            window_t *w = wm_get_bottom();
            while(w) { num_windows++; w = w->next; }
            
            /* Recalculate exact total width of taskbar items just like draw_taskbar */
            int32_t btn_width = 40;
            int32_t start_btn_width = 48;
            int32_t total_width = start_btn_width + (num_windows * (btn_width + 8));
            int32_t start_x = (s_width - total_width) / 2;
            
            if (mx >= start_x && mx <= start_x + start_btn_width) {
                s_start_menu_open = !s_start_menu_open;
            } else {
                s_start_menu_open = false;
            }
            found = true;
        } else if (s_start_menu_open && mx >= (int32_t)(s_width - 240)/2 && mx <= (int32_t)(s_width + 240)/2 && my >= (int32_t)s_height - 40 - 320 - 12 && my < (int32_t)s_height - 40) {
            /* Clicked inside start menu */
            int32_t sm_y = (int32_t)s_height - 40 - 320 - 12;
            extern void desktop_create_terminal(void);
            extern void desktop_create_explorer(void);
            extern void desktop_create_sysinfo(void);
            
            if (my >= sm_y + 48 && my < sm_y + 80) {
                desktop_create_terminal();
            } else if (my >= sm_y + 88 && my < sm_y + 120) {
                desktop_create_explorer();
            } else if (my >= sm_y + 128 && my < sm_y + 160) {
                desktop_create_sysinfo();
            } else if (my >= sm_y + 320 - 48 && my < sm_y + 320 - 16) {
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
            int32_t title_y = win->y - 24;
            /* Check title bar click */
            if (mx >= win->x && mx <= win->x + (int32_t)win->width &&
                my >= title_y && my <= win->y) {
                
                /* Check if clicked the close button (width 32) */
                if (mx >= win->x + (int32_t)win->width - 32) {
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
/* Draw Wallpaper (Gradient)                                          */
/* ------------------------------------------------------------------ */
static void draw_wallpaper(void) {
    /* A nice dark modern gradient (Win 11 style dark mode) */
    uint32_t r1 = 0x05, g1 = 0x10, b1 = 0x20;
    uint32_t r2 = 0x10, g2 = 0x30, b2 = 0x50;
    
    /* Pre-calculate gradient to save time */
    for (uint32_t y = 0; y < s_height; y++) {
        uint32_t r = r1 + ((r2 - r1) * y) / s_height;
        uint32_t g = g1 + ((g2 - g1) * y) / s_height;
        uint32_t b = b1 + ((b2 - b1) * y) / s_height;
        
        uint32_t color = (r << 16) | (g << 8) | b;
        fb_fillrect(0, y, s_width, 1, color);
    }
}

/* ------------------------------------------------------------------ */
/* Render all layers to screen                                        */
/* ------------------------------------------------------------------ */
void compositor_render(void) {
    if (!fb_available()) return;

    /* 1. Clear background (Wallpaper) */
    draw_wallpaper();

    /* 2. Draw windows back-to-front */
    window_t *win = wm_get_bottom();
    while (win) {
        draw_window(win);
        win = win->next;
    }

    /* 3. Draw Taskbar */
    draw_taskbar();

    /* 4. Draw Start Menu */
    draw_start_menu();

    /* 5. Draw mouse cursor overlay */
    draw_cursor();

    /* 6. Flip backbuffer to screen */
    fb_flip();
}
