// draw_sdl.c
#include "draw.h"
#include "tetris.h"
#include "term_compat.h"
#include "sdl_layout.h"
#include "gui.h"
#include <stdint.h>
#include <stdbool.h>

static GuiColor g_colors[BLOCK_KIND] = {
    {  0, 255, 255, 255 },  // I
    {  0,   0, 255, 255 },  // J
    { 255, 165,   0, 255 }, // L
    { 255, 255,   0, 255 }, // O
    {  0, 255,   0, 255 },  // S
    { 160,  32, 240, 255 }, // T
    { 255,   0,   0, 255 }, // Z
};

static GuiColor g_board_bg = { 18, 18, 22, 255 };
static GuiColor g_grid     = { 45, 45, 55, 255 };

static void tile_metrics(int* out_tw, int* out_th)
{
    int cw = 12, ch = 18;
    sdl_layout_get_cell_px(&cw, &ch);
    if (out_tw) *out_tw = cw;
    if (out_th) *out_th = ch;
}

static void board_origin_px(int* out_x, int* out_y)
{
    int ox = 0, oy = 0;
    sdl_layout_get_board_origin_px(&ox, &oy);
    if (out_x) *out_x = ox;
    if (out_y) *out_y = oy;
}

void init_colors(void) { }

static void draw_tile_px(int px, int py, int tw, int th, GuiColor c, bool outline)
{
    Gui* gui = term_get_gui();
    if (!gui) return;

    GuiRect r = { px, py, tw - 1, th - 1 };
    if (!outline) gui_fill_rect(gui, r, c);
    else          gui_draw_rect(gui, r, c);
}

static void draw_board_grid(void)
{
    Gui* gui = term_get_gui();
    if (!gui) return;

    int ox, oy; board_origin_px(&ox, &oy);
    int tw, th; tile_metrics(&tw, &th);

    gui_fill_rect(gui, (GuiRect){ ox, oy, BOARD_WIDTH * tw, BOARD_HEIGHT * th }, g_board_bg);

    for (int x = 0; x <= BOARD_WIDTH; x++) {
        int px = ox + x * tw;
        gui_draw_rect(gui, (GuiRect){ px, oy, 1, BOARD_HEIGHT * th }, g_grid);
    }
    for (int y = 0; y <= BOARD_HEIGHT; y++) {
        int py = oy + y * th;
        gui_draw_rect(gui, (GuiRect){ ox, py, BOARD_WIDTH * tw, 1 }, g_grid);
    }
}

void draw_board(void)
{
    Gui* gui = term_get_gui();
    if (!gui) return;

    int ox, oy; board_origin_px(&ox, &oy);
    int tw, th; tile_metrics(&tw, &th);

    draw_board_grid();

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int v = tetris_cell(y, x);
            if (v <= 0) continue;

            int type = v - 1;
            if (type < 0 || type >= BLOCK_KIND) continue;

            draw_tile_px(ox + x * tw, oy + y * th, tw, th, g_colors[type], false);
        }
    }
}

void draw_block(int y, int x, int type, int rotation)
{
    if (type < 0 || type >= BLOCK_KIND) return;

    int ox, oy; board_origin_px(&ox, &oy);
    int tw, th; tile_metrics(&tw, &th);

    for (int r = 0; r < BLOCK_SIZE; r++) {
        for (int c = 0; c < BLOCK_SIZE; c++) {
            if (!tetris_mino_cell(type, rotation, r, c)) continue;

            int by = y + r;
            int bx = x + c;
            if (by < 0 || by >= BOARD_HEIGHT || bx < 0 || bx >= BOARD_WIDTH) continue;

            draw_tile_px(ox + bx * tw, oy + by * th, tw, th, g_colors[type], false);
        }
    }
}

void draw_ghost_block(int y, int x, int type, int rotation)
{
    if (type < 0 || type >= BLOCK_KIND) return;

    int ox, oy; board_origin_px(&ox, &oy);
    int tw, th; tile_metrics(&tw, &th);
    GuiColor c = g_colors[type];
    c.a = 80;

    for (int r = 0; r < BLOCK_SIZE; r++) {
        for (int cc = 0; cc < BLOCK_SIZE; cc++) {
            if (!tetris_mino_cell(type, rotation, r, cc)) continue;

            int by = y + r;
            int bx = x + cc;
            if (by < 0 || by >= BOARD_HEIGHT || bx < 0 || bx >= BOARD_WIDTH) continue;

            draw_tile_px(ox + bx * tw, oy + by * th, tw, th, c, true);
        }
    }
}

void draw_mino_preview(int top, int left, int type, int rotation)
{
    if (type < 0 || type >= BLOCK_KIND) return;

    Gui* gui = term_get_gui();
    if (!gui) return;

    int ox = 0, oy = 0;
    sdl_layout_get_board_origin_px(&ox, &oy);

    int cw = 12, ch = 18;
    sdl_layout_get_cell_px(&cw, &ch);

    const int tw = cw / 2;
    const int th = ch / 2;

    const int px0 = ox + left * cw;
    const int py0 = oy + top  * ch;

    gui_fill_rect(gui, (GuiRect){ px0, py0, BLOCK_SIZE * tw + 8, BLOCK_SIZE * th + 8 }, (GuiColor){ 25, 25, 30, 255 });
    gui_draw_rect(gui, (GuiRect){ px0, py0, BLOCK_SIZE * tw + 8, BLOCK_SIZE * th + 8 }, (GuiColor){ 70, 70, 85, 255 });

    for (int r = 0; r < BLOCK_SIZE; r++) {
        for (int c = 0; c < BLOCK_SIZE; c++) {
            if (!tetris_mino_cell(type, rotation, r, c)) continue;
            int px = px0 + 4 + c * tw;
            int py = py0 + 4 + r * th;
            draw_tile_px(px, py, tw, th, g_colors[type], false);
        }
    }
}

void draw_line_clear_anim(const int rows[], int count, int frame)
{
    if (count <= 0) return;

    Gui* gui = term_get_gui();
    if (!gui) return;

    int ox, oy; board_origin_px(&ox, &oy);
    int tw, th; tile_metrics(&tw, &th);

    const int phase = frame % 6;
    const int a = (phase < 3) ? 255 : 120;
    GuiColor flash = { 240, 240, 255, (uint8_t)a };

    for (int i = 0; i < count; i++) {
        int ry = rows[i];
        if (ry < 0 || ry >= BOARD_HEIGHT) continue;
        gui_fill_rect(gui, (GuiRect){ ox, oy + ry * th, BOARD_WIDTH * tw, th }, flash);
    }
}