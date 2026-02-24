#include <stdio.h>
#include <string.h>

#include "scene.h"
#include "scene_manager.h"
#include "records.h"
#include "settings.h"
#include "app_context.h"

// SDL/GUI 추상화
#include "term_compat.h"
#include "gui.h"


static long page = 0;
static const int page_size = 10;

static void enter(AppContext* ctx) { (void)ctx;
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


static void render(AppContext* ctx) { (void)ctx;
    Gui* gui = term_get_gui();
    if (gui) { render_sdl(gui); return; }
}

static void render_sdl(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int lh = gui_text_height(gui);
    const int pad = 12;
    const int inner_pad = 14;
    const int line_step = lh + 4;

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

    const char* footer = "[A/←] Prev   [D/→] Next   [ESC/B] Back";

    // ===== 패널 레이아웃(텍스트 감싸기) =====
    int line_count = 0;
    const char* line0 = info;
    line_count++;

    const char* header1 = "No   Stage    Score    BlocksUsed";
    const char* header2 = "-----------------------------------";

    RecordEntry tmp[page_size];
    int n = 0;
    char rows[page_size][96];

    if (total > 0) {
        n = records_read_latest_page(page, page_size, tmp);
        line_count += 2; // header1, header2
        line_count += n; // rows
    } else {
        line_count += 1; // "No records yet."
    }

    line_count += 1; // footer

    // 최대 폭 계산
    int maxw = 0;
    int ww = gui_text_width(gui, line0); if (ww > maxw) maxw = ww;
    ww = gui_text_width(gui, footer); if (ww > maxw) maxw = ww;
    if (total > 0) {
        ww = gui_text_width(gui, header1); if (ww > maxw) maxw = ww;
        ww = gui_text_width(gui, header2); if (ww > maxw) maxw = ww;
    } else {
        ww = gui_text_width(gui, "No records yet."); if (ww > maxw) maxw = ww;
    }

    // rows 문자열 미리 만들고 폭 반영
    if (total > 0) {
        for (int i = 0; i < n; i++) {
            long global_no = total - (page * page_size + i);

            char stage_buf[8];
            if (tmp[i].stage == 0) strcpy(stage_buf, "INF");
            else snprintf(stage_buf, sizeof(stage_buf), "%d", tmp[i].stage);

            snprintf(rows[i], sizeof(rows[i]),
                     "%3ld   %5s   %7d   %10d",
                     global_no, stage_buf, tmp[i].score, tmp[i].blocks_used);

            ww = gui_text_width(gui, rows[i]);
            if (ww > maxw) maxw = ww;
        }
    }

    int panel_w = maxw + inner_pad * 2;
    if (panel_w > w - 24) panel_w = w - 24;
    int panel_h = inner_pad * 2 + (line_count * line_step) - 4;
    if (panel_h > h - 24) panel_h = h - 24;

    int px = (w - panel_w) / 2;
    int py = pad + lh + 10;
    if (py + panel_h > h - 12) py = h - 12 - panel_h;
    if (py < pad + lh + 6) py = pad + lh + 6;

    GuiColor panel_bg = (GuiColor){ 40,40,48,235 };
    GuiColor panel_bd = (GuiColor){ 180,180,190,255 };
    gui_fill_rect(gui, (GuiRect){ px, py, panel_w, panel_h }, panel_bg);
    gui_draw_rect(gui, (GuiRect){ px, py, panel_w, panel_h }, panel_bd);

    // ===== 패널 내부 텍스트 =====
    int y = py + inner_pad;
    gui_draw_text(gui, px + inner_pad, y, dim_c, info);
    y += line_step;

    if (total == 0) {
        const char* none = "No records yet.";
        gui_draw_text(gui, px + inner_pad, y, text_c, none);
        y += line_step;
    } else {
        gui_draw_text(gui, px + inner_pad, y, text_c, header1);
        y += line_step;
        gui_draw_text(gui, px + inner_pad, y, dim_c, header2);
        y += line_step;

        for (int i = 0; i < n; i++) {
            gui_draw_text(gui, px + inner_pad, y, text_c, rows[i]);
            y += line_step;
        }
    }

    gui_draw_text(gui, px + inner_pad, py + panel_h - inner_pad - lh, dim_c, footer);
}

// scene_records.c
static void handle_input(AppContext* ctx, int ch)
{
    GameSettings* settings = &ctx->settings;

    if (ch == IK_LEFT || ch == 'a' || ch == 'A' || ch == (int)settings->key_page_prev) {
        page--;
    } else if (ch == IK_RIGHT || ch == 'd' || ch == 'D' || ch == (int)settings->key_page_next) {
        page++;
    } else if (ch == IK_CANCEL || ch == 'b' || ch == 'B' || ch == (int)settings->key_back) {
        scene_set(&g_scene_title);
    }
}

static void update(AppContext* ctx, int dt_ms)
{
    (void)dt_ms;
    (void)ctx;
}
Scene g_scene_records = {
    .name = "records",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input
};