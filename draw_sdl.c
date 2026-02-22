// draw_sdl.c
#include "draw.h"
#include "tetris_data.h"
#include "term_compat.h"
#include "gui.h"
#include <stdint.h>

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
    term_get_cell_px(&cw, &ch);

    // ncurses의 “가로 2칸” 감각 유지
    if (out_tw) *out_tw = cw * 2;
    if (out_th) *out_th = ch;
}

static void board_origin_px(int* out_x, int* out_y)
{
    int cw = 12, ch = 18;
    term_get_cell_px(&cw, &ch);

    if (out_x) *out_x = cw * 2;
    if (out_y) *out_y = ch * 2;
}

void init_colors(void)
{
    // ncurses 호환용 이름 유지. SDL에선 할 일 없음.
}

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

    // grid
    for (int x = 0; x <= BOARD_WIDTH; x++) {
        int px = ox + x * tw;
        gui_fill_rect(gui, (GuiRect){ px, oy, 1, BOARD_HEIGHT * th }, g_grid);
    }
    for (int y = 0; y <= BOARD_HEIGHT; y++) {
        int py = oy + y * th;
        gui_fill_rect(gui, (GuiRect){ ox, py, BOARD_WIDTH * tw, 1 }, g_grid);
    }
}

void draw_board(int b[BOARD_HEIGHT][BOARD_WIDTH])
{
    draw_board_grid();

    int ox, oy; board_origin_px(&ox, &oy);
    int tw, th; tile_metrics(&tw, &th);

    for (int yy = 0; yy < BOARD_HEIGHT; yy++) {
        for (int xx = 0; xx < BOARD_WIDTH; xx++) {
            int v = b[yy][xx];
            if (v <= 0) continue;
            int type = v - 1;
            if (type < 0 || type >= BLOCK_KIND) continue;

            draw_tile_px(ox + xx * tw, oy + yy * th, tw, th, g_colors[type], false);
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
            if (!blocks[type][rotation][r][c]) continue;

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
    c.a = 180;

    for (int r = 0; r < BLOCK_SIZE; r++) {
        for (int cc = 0; cc < BLOCK_SIZE; cc++) {
            if (!blocks[type][rotation][r][cc]) continue;

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

    int cw = 12, ch = 18;
    term_get_cell_px(&cw, &ch);

    // preview는 작게
    const int tw = (cw * 2) / 2;
    const int th = ch / 2;

    const int px0 = left * cw;
    const int py0 = top  * ch;

    gui_fill_rect(gui, (GuiRect){ px0, py0, BLOCK_SIZE * tw + 8, BLOCK_SIZE * th + 8 }, (GuiColor){ 25, 25, 30, 255 });
    gui_draw_rect(gui, (GuiRect){ px0, py0, BLOCK_SIZE * tw + 8, BLOCK_SIZE * th + 8 }, (GuiColor){ 70, 70, 85, 255 });

    for (int r = 0; r < BLOCK_SIZE; r++) {
        for (int c = 0; c < BLOCK_SIZE; c++) {
            if (!blocks[type][rotation][r][c]) continue;
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