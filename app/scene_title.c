// app/scene_title.c
#include "scene.h"
#include "scene_manager.h"
#include "app_context.h"   // ✅ AppContext 구조체 정의(= ctx->gui 접근 가능)
#include "gui.h"
#include "titlefall.h"
#include "input_keys.h"
#include "term_compat.h" 
#include "titlefx.h"

#include <string.h>

static int cursor = 0;

static const char* menu[] = { "START", "RECORDS", "SETTINGS", "EXIT" };
static const int MENU_COUNT = (int)(sizeof(menu) / sizeof(menu[0]));

static GuiTexture* s_logo = NULL;
static int s_logo_w = 0;
static int s_logo_h = 0;

// ✅ 프로토타입(enter에서 쓰기 전에 선언
static void enter(AppContext* ctx)
{(void)ctx;
    cursor = 0;

    Gui* gui = term_get_gui();           // ✅ 이제 AppContext 완전 타입이라 접근 가능
    if (!gui) return;
    titlefx_set_enabled(1);

     if (!s_logo) {
        s_logo = gui_load_texture(gui, "assets/logo.png", &s_logo_w, &s_logo_h);

        if (!s_logo) {
            s_logo_w = s_logo_h = 0; // 실패 처리
        }
    }
}

static void update(AppContext* ctx, int dt_ms)
{
    (void)ctx;
    (void)dt_ms;
}

static void render(AppContext* ctx)
{(void)ctx;
    Gui* gui = term_get_gui();
    if (!gui) return;

    int win_w = 0, win_h = 0;
    gui_get_size(gui, &win_w, &win_h);


    int y = 50;

    // 로고
    if (s_logo && s_logo_w > 0 && s_logo_h > 0) {
        int target_w = (int)(win_w * 0.60f);
        int target_h = (int)((float)target_w * ((float)s_logo_h / (float)s_logo_w));
        GuiRect dst = { (win_w - target_w) / 2, y, target_w, target_h };
        gui_draw_texture(gui, s_logo, dst);
            int y_min = dst.y + dst.h + 8;
            int y_max = (win_h / 2) +60;   // 버튼이 너무 내려가면 여기까지만

            y = y_min;
            if (y > y_max) y = y_max;
    } else {
        // 로고 없으면 텍스트 대체
        gui_draw_text(gui, (win_w / 2) - 40, y, (GuiColor){235,235,235,255}, "TETRIS");
        y += 80;
    }

    // 메뉴 버튼
    const int item_w = 260;
    const int item_h = 46;
    const int gap    = 14;

    int total_h = MENU_COUNT * item_h + (MENU_COUNT - 1) * gap;

    int start_x = (win_w - item_w) / 2;
    int start_y = y;

    // 아래쪽만 클램프 + 로고 아래 보장
    if (start_y + total_h > win_h - 30) start_y = win_h - 30 - total_h;
    if (start_y < y) start_y = y;

    for (int i = 0; i < MENU_COUNT; i++) {
        GuiRect r = { start_x, start_y + i * (item_h + gap), item_w, item_h };

        GuiColor fill = (i == cursor)
            ? (GuiColor){240,240,240,255}
            : (GuiColor){ 80, 80, 90,255};

        gui_fill_rect(gui, r, fill);

        GuiColor tc = (i == cursor)
            ? (GuiColor){ 20, 20, 24,255}
            : (GuiColor){230,230,230,255};

        int tw = gui_text_width(gui, menu[i]);
        int th = gui_text_height(gui);
        int tx = r.x + (r.w - tw) / 2;
        int ty = r.y + (r.h - th) / 2;

        gui_draw_text(gui, tx, ty, tc, menu[i]);
    }
}

static void do_select(void)
{
    switch (cursor) {
    case 0: scene_set(&g_scene_stage_select); break;
    case 1: scene_set(&g_scene_records);      break;
    case 2: scene_set(&g_scene_settings);     break;
    case 3: scene_request_quit();             break;
    default: break;
    }
}

static void handle_input(AppContext* ctx, int ch)
{
    (void)ctx;

    if (ch == IK_UP)   cursor = (cursor - 1 + MENU_COUNT) % MENU_COUNT;
    if (ch == IK_DOWN) cursor = (cursor + 1) % MENU_COUNT;

    if (ch == IK_ENTER) do_select();
    if (ch == IK_ESC)   scene_request_quit();
}

static void exit_(AppContext* ctx)
{
    (void)ctx;
    gui_destroy_texture(&s_logo);
    s_logo_w = s_logo_h = 0;
}

Scene g_scene_title = {
    .name = "title",
    .enter = enter,
    .update = update,
    .render = render,
    .handle_input = handle_input,
    .exit = exit_,
};