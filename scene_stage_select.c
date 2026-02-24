// scene_stage_select.c  (3x4 grid box menu like scene_title style)
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "config.h"
#include "input_keys.h"

#include "term_compat.h"
#include "gui.h"

// NOTE: g_selected_stage == 0 이면 "무한(ENDLESS)" 모드
int g_selected_stage = 1;

static int cursor_choice = 1; // 1..(NUMBER_OF_STAGES+1)

// 마지막 선택지: 무한 모드
static int infinite_choice(void) { return NUMBER_OF_STAGES + 1; }
static int list_count(void) { return NUMBER_OF_STAGES + 1; }

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
    if (g_selected_stage == 0) cursor_choice = infinite_choice();
    else cursor_choice = clampi(g_selected_stage, 1, NUMBER_OF_STAGES);

    cursor_choice = clampi(cursor_choice, 1, list_count());
}

// ======= 3x4 GRID NAV =======
enum { GRID_COLS = 3, GRID_ROWS = 4 };

static void choice_to_rc(int choice, int* out_r, int* out_c)
{
    int idx = choice - 1; // 0-based
    *out_r = idx / GRID_COLS;
    *out_c = idx % GRID_COLS;
}

static int rc_to_choice_clamped(int r, int c)
{
    if (r < 0) r = 0;
    if (c < 0) c = 0;
    if (r >= GRID_ROWS) r = GRID_ROWS - 1;
    if (c >= GRID_COLS) c = GRID_COLS - 1;

    int idx = r * GRID_COLS + c; // 0-based
    int cnt = list_count();

    // 존재하지 않는 칸이면, 같은 행에서 왼쪽으로 당기기
    while (idx >= cnt && c > 0) {
        c--;
        idx = r * GRID_COLS + c;
    }

    // 그래도 없으면(행 자체가 비었으면), 위로 당기기
    while (idx >= cnt && r > 0) {
        r--;
        // 해당 행의 가장 오른쪽부터 찾기
        c = GRID_COLS - 1;
        idx = r * GRID_COLS + c;
        while (idx >= cnt && c > 0) {
            c--;
            idx = r * GRID_COLS + c;
        }
    }

    if (idx < 0) idx = 0;
    if (idx >= cnt) idx = cnt - 1;
    return idx + 1; // back to 1-based choice
}

static void move_cursor_grid(int dr, int dc)
{
    int r, c;
    choice_to_rc(cursor_choice, &r, &c);
    r += dr;
    c += dc;
    cursor_choice = rc_to_choice_clamped(r, c);
}

// ======= RENDER =======
static void render_sdl(Gui* gui)
{
    static uint32_t glow_time = 0;
    glow_time += 16;   // update에서 dt 넘겨도 되지만 단순화

    int win_w = 0, win_h = 0;
    gui_get_size(gui, &win_w, &win_h);

    const int lh  = gui_text_height(gui);
    const int pad = 8;

    GuiColor title_c = (GuiColor){ 235,235,245,255 };
    GuiColor dim_c   = (GuiColor){ 180,180,190,255 };

    GuiColor box_sel = (GuiColor){ 240,240,240,255 };
    GuiColor box_nor = (GuiColor){  80, 80,  90,255 };
    GuiColor fg_sel  = (GuiColor){  20, 20,  24,255 };
    GuiColor fg_nor  = (GuiColor){ 230,230,230,255 };

    // ===== 타이틀 =====
    {
        const char* title = "STAGE SELECT";
        int tw = gui_text_width(gui, title);
        int tx = (win_w - tw) / 2; if (tx < 0) tx = 0;
        int ty = pad;
        gui_draw_text(gui, tx, ty, title_c, title);
    }

    // ===== 도움말 =====
    {
        const char* help1 = "ARROWS: Move   ENTER/SPACE: Confirm";
        const char* help2 = "I: Infinite   B/ESC: Back";
        int h1w = gui_text_width(gui, help1);
        int h2w = gui_text_width(gui, help2);

        int base_y = pad + lh + 6;
        gui_draw_text(gui, (win_w - h1w) / 2, base_y, dim_c, help1);
        gui_draw_text(gui, (win_w - h2w) / 2, base_y + lh + 4, dim_c, help2);
    }

    // ===== 3x4 GRID BOX =====
    const int cell_w = 300;
    const int cell_h = 60;
    const int gap_x  = 16;
    const int gap_y  = 14;

    const int grid_w = GRID_COLS * cell_w + (GRID_COLS - 1) * gap_x;
    const int grid_h = GRID_ROWS * cell_h + (GRID_ROWS - 1) * gap_y;

    int start_x = (win_w - grid_w) / 2;
    int start_y = pad + lh * 3 + 110;

    if (start_x < 12) start_x = 12;
    if (start_x + grid_w > win_w - 12) start_x = win_w - 12 - grid_w;

    if (start_y + grid_h > win_h - 20) start_y = win_h - 20 - grid_h;
    if (start_y < pad + lh * 3) start_y = pad + lh * 3;

    int cnt = list_count();

    for (int choice = 1; choice <= cnt; choice++) {
        int r, c;
        choice_to_rc(choice, &r, &c);

        int x = start_x + c * (cell_w + gap_x);
        int y = start_y + r * (cell_h + gap_y);

        GuiRect rect = { x, y, cell_w, cell_h };

        const int is_cursor = (choice == cursor_choice);
        int is_cleared = 0;

        if (choice != infinite_choice())
            is_cleared = stage_is_cleared(choice);

        GuiColor fill;
        GuiColor tc;

        if (is_cursor) {
            fill = box_sel;
            tc   = fg_sel;
        }
        else if (is_cleared) {
            // ---- 펄스 밝기 계산 ----
            float t = (float)(glow_time % 1000) / 1000.0f;   // 0~1
            float s = 0.5f + 0.5f * sinf(t * 6.28318f);       // 부드러운 파동

            int base = 120;
            int glow = (int)(base + s * 80);   // 120~200

            fill = (GuiColor){ glow, glow, 60 + (int)(s*80), 255 };
            tc   = (GuiColor){ 20,20,20,255 };
        }
        else {
            fill = box_nor;
            tc   = fg_nor;
        }

        gui_fill_rect(gui, rect, fill);

        // 라벨 만들기
        char line1[64] = {0};
        char line2[96] = {0};

        if (choice == infinite_choice()) {
            snprintf(line1, sizeof(line1), "INFINITE");
            snprintf(line2, sizeof(line2), "endless");
        } else {
            int s = choice;
            const char* clear_mark = stage_is_cleared(s) ? "CLEAR" : "";
            snprintf(line1, sizeof(line1), "Stage %d", s);
            snprintf(line2, sizeof(line2), "%s %s", stage_desc(s), clear_mark);
        }

        // 선택된(고정) 항목 표시 '*'
        {
            int selected = (g_selected_stage == 0)
                ? (choice == infinite_choice())
                : (choice == g_selected_stage);

            if (selected) {
                gui_draw_text(gui, rect.x + pad, rect.y + (rect.h - lh) / 2, title_c, "*");
            }
        }

        // 텍스트 2줄 중앙 정렬
        int l1w = gui_text_width(gui, line1);
        int l2w = gui_text_width(gui, line2);

        int tx1 = rect.x + (rect.w - l1w) / 2;
        int tx2 = rect.x + (rect.w - l2w) / 2;

        // 왼쪽 별표 영역 침범 방지
        int min_tx = rect.x + pad * 3;
        if (tx1 < min_tx) tx1 = min_tx;
        if (tx2 < min_tx) tx2 = min_tx;

        int ty1 = rect.y + (rect.h - (lh * 2 + 6)) / 2;
        int ty2 = ty1 + lh + 6;

        gui_draw_text(gui, tx1, ty1, tc, line1);
        gui_draw_text(gui, tx2, ty2, tc, line2);
    }
}

static void render(void)
{
    Gui* gui = term_get_gui();
    if (!gui) return;
    render_sdl(gui);
}

// ======= INPUT =======
static void handle_input(int ch)
{
    if (ch == IK_UP)    { move_cursor_grid(-1,  0); return; }
    if (ch == IK_DOWN)  { move_cursor_grid( 1,  0); return; }
    if (ch == IK_LEFT)  { move_cursor_grid( 0, -1); return; }
    if (ch == IK_RIGHT) { move_cursor_grid( 0,  1); return; }

    // 숫자키 점프(1~10)
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
        cursor_choice = infinite_choice();
        return;
    }

    if (ch == IK_CONFIRM || ch == '\n' || ch == ' ') {
        if (cursor_choice == infinite_choice()) g_selected_stage = 0;
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

// ======= SCENE =======
Scene g_scene_stage_select = {
    .name = "stage_select",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input,
};