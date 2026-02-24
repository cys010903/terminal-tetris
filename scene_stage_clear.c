#include <string.h>
#include <stdio.h>
#include <math.h>

#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "records.h"

// SDL/GUI 추상화
#include "term_compat.h"
#include "gui.h"


extern int g_last_stage, g_last_score, g_last_blocks_used;

static void enter(void)
{
    // 스테이지 클리어 기록 저장
    records_push(g_last_stage, g_last_score, g_last_blocks_used);

    // 클리어 상태 저장(있으면)
    stage_mark_cleared(g_selected_stage);
    records_log_append_stage_clear(g_selected_stage);
}

// ===== forward =====
static void render_sdl(Gui* gui);

static void render(void)
{
    Gui* gui = term_get_gui();
    if (gui) { render_sdl(gui); return; }

}

static void render_sdl(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int lh = gui_text_height(gui);
    const int pad = 14;

    GuiColor title_c = (GuiColor){ 235,235,245,255 };
    GuiColor text_c  = (GuiColor){ 230,230,230,255 };

    GuiColor panel_bg = (GuiColor){ 40,40,48,235 };
    GuiColor panel_bd = (GuiColor){ 180,180,190,255 };

    char line1[64];
    snprintf(line1, sizeof(line1), "STAGE %d CLEAR!", g_last_stage);

    const char* l2 = "N) Next Stage";
    const char* l3 = "R) Restart";
    const char* l4 = "T) Title";
    const char* l5 = "Q) Quit";

    int y0 = h/2 - (lh*2);

    // ===== panel (텍스트를 사각형으로 감싸기) =====
    int maxw = 0;
    int w1 = gui_text_width(gui, line1); if (w1 > maxw) maxw = w1;
    int w2 = gui_text_width(gui, l2);    if (w2 > maxw) maxw = w2;
    int w3 = gui_text_width(gui, l3);    if (w3 > maxw) maxw = w3;
    int w4 = gui_text_width(gui, l4);    if (w4 > maxw) maxw = w4;
    int w5 = gui_text_width(gui, l5);    if (w5 > maxw) maxw = w5;

    int panel_w = maxw + pad * 2;
    int panel_h = lh * 6 + pad * 2;
    int px = (w - panel_w) / 2;
    int py = y0 - pad;
    if (px < 12) px = 12;
    if (px + panel_w > w - 12) px = w - 12 - panel_w;
    if (py < 12) py = 12;
    if (py + panel_h > h - 12) py = h - 12 - panel_h;

    gui_fill_rect(gui, (GuiRect){ px, py, panel_w, panel_h }, panel_bg);
    gui_draw_rect(gui, (GuiRect){ px, py, panel_w, panel_h }, panel_bd);

    int x1 = (w - gui_text_width(gui, line1)) / 2; if (x1 < 0) x1 = 0;
    int x2 = (w - gui_text_width(gui, l2)) / 2; if (x2 < 0) x2 = 0;
    int x3 = (w - gui_text_width(gui, l3)) / 2; if (x3 < 0) x3 = 0;
    int x4 = (w - gui_text_width(gui, l4)) / 2; if (x4 < 0) x4 = 0;
    int x5 = (w - gui_text_width(gui, l5)) / 2; if (x5 < 0) x5 = 0;

    gui_draw_text(gui, x1, y0 + lh*0, title_c, line1);
    gui_draw_text(gui, x2, y0 + lh*2, text_c,  l2);
    gui_draw_text(gui, x3, y0 + lh*3, text_c,  l3);
    gui_draw_text(gui, x4, y0 + lh*4, text_c,  l4);
    gui_draw_text(gui, x5, y0 + lh*5, text_c,   l5);
}

static void handle_input(int ch)
{
    if (ch == 'n' || ch == 'N') {
        int next = g_last_stage + 1;
        if (next > NUMBER_OF_STAGES) next = NUMBER_OF_STAGES;
        g_selected_stage = next;         // stage_select에서 쓰는 전역
        scene_set(&g_scene_game);
        return;
    }

    if (ch == 'r' || ch == 'R') { scene_set(&g_scene_game); return; }
    if (ch == 't' || ch == 'T') { scene_set(&g_scene_title); return; }
    if (ch == 'q' || ch == 'Q') { scene_request_quit(); return; }
}

static void update(int dt_ms) { (void)dt_ms; }

Scene g_scene_stage_clear = {
    .name = "stage_clear",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input,
};