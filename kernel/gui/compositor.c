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

    /* Draw window title bar */
    if (win->y >= 16) {
        int32_t title_y = win->y - 16;
        
        /* Check if it's the top window for color */
        uint32_t tb_color = (win == wm_get_top()) ? 0x000055AA : 0x00444444;

        /* Draw title bar background */
        fb_fillrect(win->x, title_y, win->width, 16, tb_color);

        /* Draw close button (red X box) */
        fb_fillrect(win->x + win->width - 16, title_y, 16, 16, 0x00CC3333);
        font_draw_char(win->x + win->width - 12, title_y + 4, 'X', 0x00FFFFFF, 0x00000000);

        /* Draw title text */
        font_draw_string(win->x + 4, title_y + 4, win->title, 0x00FFFFFF, 0x00000000);
    }

    /* Draw window content buffer */
    if (win->buffer) {
        fb_blit(win->x, win->y, win->width, win->height, win->buffer);
    }
}

/* ------------------------------------------------------------------ */
/* Draw a cursor cross                                                */
/* ------------------------------------------------------------------ */
static void draw_cursor(void) {
    int32_t cx = mouse_x();
    int32_t cy = mouse_y();
    uint32_t cursor_color = 0x00FFFFFF; /* White cursor */

    /* Small cross */
    for (int i = -4; i <= 4; i++) {
        fb_putpixel(cx + i, cy, cursor_color);
        fb_putpixel(cx, cy + i, cursor_color);
    }
}

/* ------------------------------------------------------------------ */
/* Draw taskbar                                                       */
/* ------------------------------------------------------------------ */
static void draw_taskbar(void) {
    int32_t tb_height = 24;
    int32_t tb_y = s_height - tb_height;
    
    /* Taskbar background */
    fb_fillrect(0, tb_y, s_width, tb_height, 0x00222222);

    /* Start button */
    fb_fillrect(0, tb_y, 80, tb_height, 0x001155AA);
    font_draw_string(16, tb_y + 8, "Genesi", 0x00FFFFFF, 0x00000000);

    /* Draw buttons for windows */
    window_t *win = wm_get_bottom();
    int32_t btn_x = 84;
    while (win) {
        uint32_t btn_color = (win == wm_get_top()) ? 0x00444444 : 0x00333333;
        fb_fillrect(btn_x, tb_y + 2, 120, tb_height - 4, btn_color);
        font_draw_string(btn_x + 8, tb_y + 8, win->title, 0x00DDDDDD, 0x00000000);
        btn_x += 124;
        win = win->next;
    }

    /* Clock/Status area */
    font_draw_string(s_width - 64, tb_y + 8, "v0.2", 0x00888888, 0x00000000);
}

/* ------------------------------------------------------------------ */
/* Draw Start Menu                                                    */
/* ------------------------------------------------------------------ */
static void draw_start_menu(void) {
    if (!s_start_menu_open) return;
    
    int32_t sm_width = 150;
    int32_t sm_height = 100;
    int32_t sm_x = 0;
    int32_t sm_y = s_height - 24 - sm_height;
    
    /* Menu background */
    fb_fillrect(sm_x, sm_y, sm_width, sm_height, 0x00333333);
    
    /* Menu items */
    /* Item 1: Terminal */
    fb_fillrect(sm_x + 4, sm_y + 4, sm_width - 8, 24, 0x00444444);
    font_draw_string(sm_x + 12, sm_y + 12, "Terminal", 0x00FFFFFF, 0x00000000);
    
    /* Item 2: File Explorer */
    fb_fillrect(sm_x + 4, sm_y + 32, sm_width - 8, 24, 0x00444444);
    font_draw_string(sm_x + 12, sm_y + 40, "File Explorer", 0x00FFFFFF, 0x00000000);
    
    /* Item 3: System Info */
    fb_fillrect(sm_x + 4, sm_y + 60, sm_width - 8, 24, 0x00444444);
    font_draw_string(sm_x + 12, sm_y + 68, "System Info", 0x00FFFFFF, 0x00000000);
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
        if (my >= (int32_t)s_height - 24) {
            /* Click on taskbar */
            if (mx >= 0 && mx <= 80) {
                s_start_menu_open = !s_start_menu_open;
            } else {
                s_start_menu_open = false;
            }
            found = true;
        } else if (s_start_menu_open && mx >= 0 && mx <= 150 && my >= (int32_t)s_height - 24 - 100 && my < (int32_t)s_height - 24) {
            /* Clicked inside start menu */
            int32_t sm_y = (int32_t)s_height - 24 - 100;
            extern void desktop_create_terminal(void);
            extern void desktop_create_explorer(void);
            extern void desktop_create_sysinfo(void);
            
            if (my >= sm_y + 4 && my < sm_y + 28) {
                desktop_create_terminal();
            } else if (my >= sm_y + 32 && my < sm_y + 56) {
                desktop_create_explorer();
            } else if (my >= sm_y + 60 && my < sm_y + 84) {
                desktop_create_sysinfo();
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
            int32_t title_y = win->y - 16;
            /* Check title bar click */
            if (mx >= win->x && mx <= win->x + (int32_t)win->width &&
                my >= title_y && my <= win->y) {
                
                /* Check if clicked the close button */
                if (mx >= win->x + (int32_t)win->width - 16) {
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
    if (!fb_available()) return;

    /* 1. Clear background (Wallpaper) */
    fb_clear(0x00111122); /* Dark blue background */

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
