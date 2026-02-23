#include <string.h>
#include <stdio.h>

#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "config.h"
#include "input_keys.h"

// SDL/GUI 추상화
#include "term_compat.h"
#include "gui.h"

// 기본 스테이지(프로그램 시작 시 1)
// NOTE: g_selected_stage == 0 이면 "무한(ENDLESS)" 모드로 취급합니다.
int g_selected_stage = 1;

static const int kInfiniteChoice = NUMBER_OF_STAGES + 1;
static int cursor_choice = 1; // 1..NUMBER_OF_STAGES : 스테이지, (NUMBER_OF_STAGES+1) : 무한모드

static const char* stage_desc(int stage)
{
    switch (stage) {
    case 1:  return "stage1";
    case 2:  return "stage2";
    case 3:  return "stage3";
    case 4:  return "stage4";
    case 5:  return "stage5";
    case 6:  return "stage6";
    case 7:  return "stage7";
    case 8:  return "stage8";
    case 9:  return "stage9";
    case 10: return "stage10";
    default: return "";
    }
}

static void enter(void)
{
    if (g_selected_stage == 0) cursor_choice = kInfiniteChoice;
    else cursor_choice = clampi(g_selected_stage, 1, NUMBER_OF_STAGES);
    cursor_choice = clampi(cursor_choice, 1, kInfiniteChoice);
}

static void render_sdl(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int lh = gui_text_height(gui);
    const int pad = 8;

    GuiColor title_c = (GuiColor){ 235,235,245,255 };
    GuiColor text_c  = (GuiColor){ 230,230,230,255 };
    GuiColor dim_c   = (GuiColor){ 180,180,190,255 };
    GuiColor hi_bg   = (GuiColor){ 230,230,230,255 };
    GuiColor hi_fg   = (GuiColor){ 30,30,35,255 };

    // 제목
    const char* title = "STAGE SELECT";
    int tw = gui_text_width(gui, title);
    int tx = (w - tw) / 2; if (tx < 0) tx = 0;
    int ty = pad;
    gui_draw_text(gui, tx, ty, title_c, title);

    // 도움말
    const char* help1 = "UP/DOWN: Move   ENTER/SPACE: Confirm   I: Infinite";
    const char* help2 = "B/ESC: Back to Title";
    int h1w = gui_text_width(gui, help1);
    int h2w = gui_text_width(gui, help2);
    gui_draw_text(gui, (w - h1w) / 2, ty + lh + 6, dim_c, help1);
    gui_draw_text(gui, (w - h2w) / 2, ty + lh * 2 + 10, dim_c, help2);

    // 리스트 시작 y
    int list_count = NUMBER_OF_STAGES + 1;
    int list_top = (h / 2) - (list_count * lh) / 2;
    if (list_top < ty + lh * 3) list_top = ty + lh * 3;

    // ===== 일반 스테이지 =====
    for (int s = 1; s <= NUMBER_OF_STAGES; s++) {
        char line[96];
        const char* clear_mark = stage_is_cleared(s) ? "  [CLEAR]" : "";
        snprintf(line, sizeof(line), "Stage %d  (%s)%s", s, stage_desc(s), clear_mark);

        int lw = gui_text_width(gui, line);
        int x0 = (w - lw) / 2; if (x0 < 0) x0 = 0;
        int y0 = list_top + (s - 1) * lh;

        if (s == cursor_choice) {
            gui_fill_rect(gui, (GuiRect){ x0 - pad, y0 - 2, lw + pad * 2, lh + 4 }, hi_bg);
            gui_draw_text(gui, x0, y0, hi_fg, line);
        } else {
            gui_draw_text(gui, x0, y0, text_c, line);
        }

        if (s == g_selected_stage) {
            gui_draw_text(gui, x0 - pad * 2, y0, title_c, "*");
        }
    }

    // ===== 무한 모드 =====
    {
        const char* line = "INFINITE MODE  (endless)";
        int lw = gui_text_width(gui, line);
        int x0 = (w - lw) / 2; if (x0 < 0) x0 = 0;
        int y0 = list_top + NUMBER_OF_STAGES * lh;

        if (cursor_choice == kInfiniteChoice) {
            gui_fill_rect(gui, (GuiRect){ x0 - pad, y0 - 2, lw + pad * 2, lh + 4 }, hi_bg);
            gui_draw_text(gui, x0, y0, hi_fg, line);
        } else {
            gui_draw_text(gui, x0, y0, text_c, line);
        }

        if (g_selected_stage == 0) {
            gui_draw_text(gui, x0 - pad * 2, y0, title_c, "*");
        }
    }
}

static void render(void)
{
    Gui* gui = term_get_gui();
    if (!gui) return; // SDL 빌드에서는 항상 있어야 함
    render_sdl(gui);
}

static void handle_input(int ch)
{
    if (ch == IK_UP) {
        cursor_choice = clampi(cursor_choice - 1, 1, kInfiniteChoice);
        return;
    }
    if (ch == IK_DOWN) {
        cursor_choice = clampi(cursor_choice + 1, 1, kInfiniteChoice);
        return;
    }

    if (ch >= '1' && ch <= '9') {
        int s = ch - '0';
        if (s <= NUMBER_OF_STAGES) cursor_choice = s;
        return;
    }
    if (ch == '0' && NUMBER_OF_STAGES >= 10) {
        cursor_choice = 10;
        return;
    }

    if (ch == 'i' || ch == 'I') {
        cursor_choice = kInfiniteChoice;
        return;
    }

    if (ch == IK_CONFIRM || ch == '\n' || ch == ' ') {
        if (cursor_choice == kInfiniteChoice) g_selected_stage = 0;
        else g_selected_stage = clampi(cursor_choice, 1, NUMBER_OF_STAGES);

        scene_set(&g_scene_game);
        return;
    }

    if (ch == 'b' || ch == 'B' || ch == IK_CANCEL) {
        scene_set(&g_scene_title);
        return;
    }
}

static void update(int dt_ms) { (void)dt_ms; }

Scene g_scene_stage_select = {
    .name = "stage_select",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input,
};