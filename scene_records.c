#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "scene.h"
#include "scene_manager.h"
#include "records.h"

static long page = 0;
static const int page_size = 10;

static void enter(void)
{
    page = 0;
}

static void clamp_page(long total)
{
    if (total <= 0) {
        page = 0;
        return;
    }

    long max_page = (total - 1) / page_size;

    if (page < 0) page = 0;
    if (page > max_page) page = max_page;
}

static void render_list_header(int y)
{
    mvprintw(y,   4, "No  Stage   Score   BlocksUsed");
    mvprintw(y+1, 4, "--------------------------------");
}

static void render(void)
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
            global_no,
            stage_buf,
            tmp[i].score,
            tmp[i].blocks_used);
    }

    mvprintw(h - 2, 2, "[A/←] Prev  [D/→] Next  [ESC] Back");

    refresh();
}

static void handle_input(int ch)
{
    if (ch == 'a' || ch == KEY_LEFT)
        page--;
    else if (ch == 'd' || ch == KEY_RIGHT)
        page++;
    else if (ch == 27 || ch == 'b' || ch == 'B')
        scene_set(&g_scene_title);
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