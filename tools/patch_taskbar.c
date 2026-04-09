#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE *f = fopen("kernel/gui/compositor.c", "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = 0;
    fclose(f);

    // Substituir draw_taskbar inteiro
    char *start = strstr(content, "static void draw_taskbar(void) {");
    char *end = strstr(start, "/* Draw System Tray");
    
    char new_code[] = 
"static void draw_icon(int32_t x, int32_t y, const uint32_t *icon_data, uint32_t icon_w, uint32_t icon_h) {\n"
"    extern uint32_t *fb_get_backbuffer(void);\n"
"    uint32_t *bb = fb_get_backbuffer();\n"
"    if (!bb) return;\n"
"    \n"
"    int32_t scaled_w = icon_w * g_ui_scale;\n"
"    int32_t scaled_h = icon_h * g_ui_scale;\n"
"    \n"
"    for (int32_t iy = 0; iy < scaled_h; iy++) {\n"
"        for (int32_t ix = 0; ix < scaled_w; ix++) {\n"
"            if (x + ix < 0 || x + ix >= (int32_t)s_width || y + iy < 0 || y + iy >= (int32_t)s_height) continue;\n"
"            \n"
"            uint32_t src_x = (ix << 8) / g_ui_scale;\n"
"            uint32_t src_y = (iy << 8) / g_ui_scale;\n"
"            \n"
"            uint32_t x0 = src_x >> 8;\n"
"            uint32_t y0 = src_y >> 8;\n"
"            uint32_t x1 = x0 + 1;\n"
"            uint32_t y1 = y0 + 1;\n"
"            if (x1 >= icon_w) x1 = icon_w - 1;\n"
"            if (y1 >= icon_h) y1 = icon_h - 1;\n"
"            \n"
"            uint32_t fx = src_x & 0xFF;\n"
"            uint32_t fy = src_y & 0xFF;\n"
"            \n"
"            uint32_t c00 = icon_data[y0 * icon_w + x0];\n"
"            uint32_t c01 = icon_data[y0 * icon_w + x1];\n"
"            uint32_t c10 = icon_data[y1 * icon_w + x0];\n"
"            uint32_t c11 = icon_data[y1 * icon_w + x1];\n"
"            \n"
"            uint32_t a00 = (c00 >> 24) & 0xFF, a01 = (c01 >> 24) & 0xFF;\n"
"            uint32_t a10 = (c10 >> 24) & 0xFF, a11 = (c11 >> 24) & 0xFF;\n"
"            uint32_t top_a = (a00 * (256 - fx) + a01 * fx) >> 8;\n"
"            uint32_t bot_a = (a10 * (256 - fx) + a11 * fx) >> 8;\n"
"            uint32_t alpha = (top_a * (256 - fy) + bot_a * fy) >> 8;\n"
"            \n"
"            if (alpha > 0) {\n"
"                uint32_t r00 = (c00 >> 16) & 0xFF, r01 = (c01 >> 16) & 0xFF;\n"
"                uint32_t r10 = (c10 >> 16) & 0xFF, r11 = (c11 >> 16) & 0xFF;\n"
"                uint32_t top_r = (r00 * (256 - fx) + r01 * fx) >> 8;\n"
"                uint32_t bot_r = (r10 * (256 - fx) + r11 * fx) >> 8;\n"
"                uint32_t fr = (top_r * (256 - fy) + bot_r * fy) >> 8;\n"
"                \n"
"                uint32_t g00 = (c00 >> 8) & 0xFF, g01 = (c01 >> 8) & 0xFF;\n"
"                uint32_t g10 = (c10 >> 8) & 0xFF, g11 = (c11 >> 8) & 0xFF;\n"
"                uint32_t top_g = (g00 * (256 - fx) + g01 * fx) >> 8;\n"
"                uint32_t bot_g = (g10 * (256 - fx) + g11 * fx) >> 8;\n"
"                uint32_t fg = (top_g * (256 - fy) + bot_g * fy) >> 8;\n"
"                \n"
"                uint32_t b00 = c00 & 0xFF, b01 = c01 & 0xFF;\n"
"                uint32_t b10 = c10 & 0xFF, b11 = c11 & 0xFF;\n"
"                uint32_t top_b = (b00 * (256 - fx) + b01 * fx) >> 8;\n"
"                uint32_t bot_b = (b10 * (256 - fx) + b11 * fx) >> 8;\n"
"                uint32_t fb = (top_b * (256 - fy) + bot_b * fy) >> 8;\n"
"                \n"
"                if (alpha == 255) {\n"
"                    bb[(y + iy) * s_width + (x + ix)] = (fr << 16) | (fg << 8) | fb;\n"
"                } else {\n"
"                    uint32_t bg = bb[(y + iy) * s_width + (x + ix)];\n"
"                    uint32_t bgr = (bg >> 16) & 0xFF;\n"
"                    uint32_t bgg = (bg >> 8) & 0xFF;\n"
"                    uint32_t bgb = bg & 0xFF;\n"
"                    \n"
"                    uint32_t r = (fr * alpha + bgr * (255 - alpha)) / 255;\n"
"                    uint32_t g = (fg * alpha + bgg * (255 - alpha)) / 255;\n"
"                    uint32_t b = (fb * alpha + bgb * (255 - alpha)) / 255;\n"
"                    \n"
"                    bb[(y + iy) * s_width + (x + ix)] = (r << 16) | (g << 8) | b;\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"}\n"
"\n"
"#include \"icons/icon_grid4.h\"\n"
"#include \"icons/icon_grid9.h\"\n"
"#include \"icons/icon_search.h\"\n"
"#include \"icons/icon_folder.h\"\n"
"#include \"icons/icon_cmd.h\"\n"
"#include \"icons/icon_power.h\"\n"
"\n"
"static void draw_taskbar(void) {\n"
"    int32_t tb_height = 64 * g_ui_scale;\n"
"    int32_t tb_width = 460 * g_ui_scale;\n"
"    int32_t tb_x = (s_width - tb_width) / 2;\n"
"    int32_t tb_y = s_height - tb_height - (20 * g_ui_scale); /* Floating */\n"
"    \n"
"    /* Taskbar background (Dark frosted glass, matching design) */\n"
"    draw_rounded_rect(tb_x, tb_y, tb_width, tb_height, 32 * g_ui_scale, 0xC02A2E33, true);\n"
"    \n"
"    /* Calculate icon spacing */\n"
"    int32_t item_count = 6;\n"
"    int32_t icon_size = 32 * g_ui_scale;\n"
"    int32_t padding = (tb_width - (item_count * icon_size)) / (item_count + 1);\n"
"    \n"
"    int32_t cur_x = tb_x + padding;\n"
"    int32_t icon_y = tb_y + (tb_height - icon_size) / 2;\n"
"    \n"
"    /* 1. Grid 4 (Start Menu) */\n"
"    draw_icon(cur_x, icon_y, icon_grid4, ICON_GRID4_WIDTH, ICON_GRID4_HEIGHT);\n"
"    cur_x += icon_size + padding;\n"
"    \n"
"    /* 2. Grid 9 (App Drawer) */\n"
"    draw_icon(cur_x, icon_y, icon_grid9, ICON_GRID9_WIDTH, ICON_GRID9_HEIGHT);\n"
"    cur_x += icon_size + padding;\n"
"    \n"
"    /* 3. Search */\n"
"    draw_icon(cur_x, icon_y, icon_search, ICON_SEARCH_WIDTH, ICON_SEARCH_HEIGHT);\n"
"    cur_x += icon_size + padding;\n"
"    \n"
"    /* 4. Folder (with Blue Circle background) */\n"
"    int32_t circle_size = 48 * g_ui_scale;\n"
"    draw_rounded_rect(cur_x + (icon_size - circle_size)/2, tb_y + (tb_height - circle_size)/2, circle_size, circle_size, circle_size/2, 0xFF3B82F6, true);\n"
"    draw_icon(cur_x, icon_y, icon_folder, ICON_FOLDER_WIDTH, ICON_FOLDER_HEIGHT);\n"
"    cur_x += icon_size + padding;\n"
"    \n"
"    /* 5. CMD / Settings */\n"
"    draw_icon(cur_x, icon_y, icon_cmd, ICON_CMD_WIDTH, ICON_CMD_HEIGHT);\n"
"    cur_x += icon_size + padding;\n"
"    \n"
"    /* 6. Power */\n"
"    draw_icon(cur_x, icon_y, icon_power, ICON_POWER_WIDTH, ICON_POWER_HEIGHT);\n"
"}\n\n";

    int start_idx = start - content;
    int end_idx = end - content;

    FILE *out = fopen("kernel/gui/compositor.c", "w");
    fwrite(content, 1, start_idx, out);
    fwrite(new_code, 1, strlen(new_code), out);
    fwrite(content + end_idx, 1, size - end_idx, out);
    fclose(out);
    free(content);
    return 0;
}