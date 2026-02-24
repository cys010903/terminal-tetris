#include "draw_game.h"
#include "layout.h"
#include "draw.h"
#include "tetris.h"
#include "settings.h"
#include "term_compat.h"

#ifndef BUILD_SDL
#include <ncursesw/ncurses.h>
#else
#include "gui.h"
#endif

static void draw_hud(const GameView* v, const HudLayout* l)
{
#ifdef BUILD_SDL
    Gui* gui = term_get_gui();
    if (!gui) return;

    int cw = 12, ch = 18;
    term_get_cell_px(&cw, &ch);

    int ox = 0, oy = 0;
    term_get_board_origin_px(&ox, &oy);

    // ===== HOLD =====
    if (v->hold_enabled) {
        gui_draw_text(gui,
            ox + l->hold_x_cell * cw,
            oy + l->hold_y_cell * ch,
            (GuiColor){235,235,245,255},
            "HOLD"
        );

        if (v->hold_type >= 0) {
            draw_mino_preview(l->hold_y_cell + 2, l->hold_x_cell, v->hold_type, 0);
        }
    }

    // NEXT
    gui_draw_text(gui, ox + l->ui_left_cell * cw, oy + 1 * ch, (GuiColor){235,235,245,255}, "NEXT");
    for (int i = 0; i < 5; i++) {
        int top  = l->next_top_cell + i * BLOCK_SIZE;
        int left = l->ui_left_cell;
        draw_mino_preview(top, left, v->next_queue[i], 0);
    }

    // INFO
    char buf[128];
    int px = ox + l->info_x_cell * cw;
    int py = oy + l->info_y_cell * ch;

    snprintf(buf, sizeof(buf), "STAGE : %d", v->stage);
    gui_draw_text(gui, px - 6*cw, py + 0*ch, (GuiColor){235,235,245,255}, buf);

    snprintf(buf, sizeof(buf), "SCORE : %d", v->score);
    gui_draw_text(gui, px - 6*cw, py + 1*ch, (GuiColor){235,235,245,255}, buf);

    if (v->infinite_mode) {
        snprintf(buf, sizeof(buf), "LEVEL : %d", v->level);
        gui_draw_text(gui, px - 6*cw, py + 2*ch, (GuiColor){235,235,245,255}, buf);
        snprintf(buf, sizeof(buf), "LINES : %d", v->lines_total);
        gui_draw_text(gui, px - 6*cw, py + 3*ch, (GuiColor){235,235,245,255}, buf);
    } else {
        snprintf(buf, sizeof(buf), "GOAL  : %d", v->goal);
        gui_draw_text(gui, px - 6*cw, py + 2*ch, (GuiColor){235,235,245,255}, buf);
    }

#else
    mvaddstr(1, l->ui_left_col, "NEXT");
    for (int i = 0; i < 5; i++) {
        int top = l->next_top_row + i * BLOCK_SIZE;
        draw_mino_preview(top, l->ui_left_col, v->next_queue[i], 0);
    }

    mvprintw(l->info_y_row + 0, l->info_x_col, "STAGE : %d", v->stage);
    mvprintw(l->info_y_row + 1, l->info_x_col, "SCORE : %d", v->score);

    if (v->infinite_mode) {
        mvprintw(l->info_y_row + 2, l->info_x_col, "LEVEL : %d", v->level);
        mvprintw(l->info_y_row + 3, l->info_x_col, "LINES : %d", v->lines_total);
    } else {
        mvprintw(l->info_y_row + 2, l->info_x_col, "GOAL  : %d", v->goal);
    }
#endif
}

void draw_game(const GameView* v)
{
    if (!v) return;

#ifndef BUILD_SDL
    erase();
#endif

    draw_board(v->board);

    if (v->mode == GAMEVIEW_LINECLEAR && v->lc_count > 0) {
        draw_line_clear_anim(v->lc_rows, v->lc_count, v->lc_frame);
    }

    if (v->mode == GAMEVIEW_PLAY) {
        if (v->ghost_enabled) {
            draw_ghost_block(v->ghost_y, v->cur_x, v->cur_type, v->cur_rot);
        }
        draw_block(v->cur_y, v->cur_x, v->cur_type, v->cur_rot);
    }

    HudLayout l;
    layout_build(&l);
    draw_hud(v, &l);

#ifndef BUILD_SDL
    refresh();
#endif
}