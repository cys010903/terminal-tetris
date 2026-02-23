#include <stdio.h>
#include <string.h>

#include "scene.h"
#include "scene_manager.h"
#include "records.h"
#include "settings.h"

// SDL/GUI 추상화
#include "term_compat.h"
#include "gui.h"

#ifndef BUILD_SDL
#include <ncurses.h>
#endif

static long page = 0;
static const int page_size = 10;

static void enter(void)
{
    page = 0;
}

static void clamp_page(long total)
{
    if (total <= 0) { page = 0; return; }

    long max_page = (total - 1) / page_size;
    if (page < 0) page = 0;
    if (page > max_page) page = max_page;
}

// ===== forward =====
static void render_sdl(Gui* gui);
#ifndef BUILD_SDL
static void render_ncu(void);
#endif

static void render(void)
{
    Gui* gui = term_get_gui();
    if (gui) { render_sdl(gui); return; }

#ifndef BUILD_SDL
    render_ncu();
#endif
}

static void render_sdl(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int lh = gui_text_height(gui);
    const int pad = 12;

    GuiColor title_c = (GuiColor){ 235,235,245,255 };
    GuiColor text_c  = (GuiColor){ 230,230,230,255 };
    GuiColor dim_c   = (GuiColor){ 180,180,190,255 };

    // title
    const char* title = "RECORDS";
    int tw = gui_text_width(gui, title);
    int tx = (w - tw) / 2; if (tx < 0) tx = 0;
    gui_draw_text(gui, tx, pad, title_c, title);

    long total = records_log_count();
    clamp_page(total);

    long total_pages = (total <= 0) ? 0 : ((total - 1) / page_size) + 1;

    char info[64];
    snprintf(info, sizeof(info), "Total %ld   Page %ld / %ld",
             total,
             (total == 0) ? 0 : (page + 1),
             total_pages);
    gui_draw_text(gui, pad, pad + lh + 6, dim_c, info);

    if (total == 0) {
        const char* none = "No records yet.";
        int nx = (w - gui_text_width(gui, none)) / 2; if (nx < 0) nx = 0;
        gui_draw_text(gui, nx, h/2, text_c, none);

        const char* footer = "[A/←] Prev   [D/→] Next   [ESC/B] Back";
        int fx = (w - gui_text_width(gui, footer)) / 2; if (fx < 0) fx = 0;
        gui_draw_text(gui, fx, h - pad - lh, dim_c, footer);
        return;
    }

    // header
    int y = pad + lh*2 + 16;
    gui_draw_text(gui, pad, y, text_c, "No   Stage    Score    BlocksUsed");
    y += lh;
    gui_draw_text(gui, pad, y, dim_c, "-----------------------------------");
    y += lh;

    RecordEntry tmp[page_size];
    int n = records_read_latest_page(page, page_size, tmp);

    for (int i = 0; i < n; i++) {
        long global_no = total - (page * page_size + i);

        char stage_buf[8];
        if (tmp[i].stage == 0) strcpy(stage_buf, "INF");
        else snprintf(stage_buf, sizeof(stage_buf), "%d", tmp[i].stage);

        char line[96];
        snprintf(line, sizeof(line),
                 "%3ld   %5s   %7d   %10d",
                 global_no, stage_buf, tmp[i].score, tmp[i].blocks_used);

        gui_draw_text(gui, pad, y + i * lh, text_c, line);
    }

    // footer
    const char* footer = "[A/←] Prev   [D/→] Next   [ESC/B] Back";
    int fx = (w - gui_text_width(gui, footer)) / 2; if (fx < 0) fx = 0;
    gui_draw_text(gui, fx, h - pad - lh, dim_c, footer);
}

#ifndef BUILD_SDL
static void render_list_header(int y)
{
    mvprintw(y,   4, "No  Stage   Score   BlocksUsed");
    mvprintw(y+1, 4, "--------------------------------");
}

static void render_ncu(void)
{
    erase();

    int h, w;
    getmaxyx(stdscr, h, w);

    mvprintw(1, (w - 14) / 2, "=== RECORDS ===");

    long total = records_log_count();
    clamp_page(total);

    long total_pages = (total <= 0)
        ? 0
        : ((total - 1) / page_size) + 1;

    mvprintw(2, 2, "Total %ld   Page %ld / %ld",
             total,
             (total == 0) ? 0 : page + 1,
             total_pages);

    if (total == 0) {
        mvprintw(h/2, (w - 16)/2, "No records yet.");
        refresh();
        return;
    }

    RecordEntry tmp[page_size];
    int n = records_read_latest_page(page, page_size, tmp);

    int start_y = 4;
    render_list_header(start_y);
    start_y += 2;

    for (int i = 0; i < n; i++)
    {
        long global_no = total - (page * page_size + i);

        char stage_buf[8];
        if (tmp[i].stage == 0) strcpy(stage_buf, "INF");
        else snprintf(stage_buf, sizeof(stage_buf), "%d", tmp[i].stage);

        mvprintw(start_y + i, 4,
            "%3ld  %5s  %7d  %10d",
            global_no, stage_buf, tmp[i].score, tmp[i].blocks_used);
    }

    mvprintw(h - 2, 2, "[A/←] Prev  [D/→] Next  [ESC] Back");
    refresh();
}
#endif

// scene_records.c
static void handle_input(int ch)
{
    if (ch == IK_LEFT || ch == 'a' || ch == 'A' || ch == g_settings.key_page_prev) {
        page--;
    } else if (ch == IK_RIGHT || ch == 'd' || ch == 'D' || ch == g_settings.key_page_next) {
        page++;
    } else if (ch == IK_CANCEL || ch == 'b' || ch == 'B' || ch == g_settings.key_back) {
        scene_set(&g_scene_title);
    }
}

static void update(int dt_ms){ (void)dt_ms; }

Scene g_scene_records = {
    .name = "records",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input
};