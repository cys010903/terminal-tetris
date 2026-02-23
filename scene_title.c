// scene_title.c (SDL-only, no ncurses, no SDL_image)
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include "scene.h"
#include "scene_manager.h"
#include "term_compat.h"
#include "gui.h"
#include "input_keys.h"
#include "titlefall.h"
#include <SDL2/SDL.h>
#include <string.h>

static int cursor = 0;

static const char* menu[] = { "START", "RECORDS", "SETTINGS", "EXIT" };
static const int MENU_COUNT = (int)(sizeof(menu) / sizeof(menu[0]));

static void enter(void)
{
    cursor = 0;
}

static void update(int dt_ms)
{
    (void)dt_ms;
}

static void draw_center_text(Gui* gui, int y, GuiColor c, const char* s)
{
    int w, h;
    gui_get_size(gui, &w, &h);

    int tw = gui_text_width(gui, s);
    int x = (w - tw) / 2;
    if (x < 0) x = 0;

    gui_draw_text(gui, x, y, c, s);
}

static void render(void)
{
    Gui* gui = term_get_gui();
    if (!gui) return;

    int win_w = 0, win_h = 0;
    gui_get_size(gui, &win_w, &win_h);

    // 배경: main loop의 begin/end frame 사이에서 직접 clear
    SDL_SetRenderDrawColor(gui->ren, 20, 20, 24, 255);


    // 타이틀(텍스트 로고)
    draw_center_text(gui, 70, (GuiColor){ 235,235,235,255 }, "TETRIS");

    // 메뉴 레이아웃
    const int item_w = 260;
    const int item_h = 46;
    const int gap    = 14;

    int total_h = MENU_COUNT * item_h + (MENU_COUNT - 1) * gap;
    int start_y = 150;
    if (start_y + total_h > win_h - 30) start_y = win_h - 30 - total_h;
    if (start_y < 110) start_y = 110;

    int start_x = (win_w - item_w) / 2;

    for (int i = 0; i < MENU_COUNT; i++) {
        GuiRect r = { start_x, start_y + i * (item_h + gap), item_w, item_h };

        GuiColor fill = (i == cursor)
            ? (GuiColor){ 240, 240, 240, 255 }
            : (GuiColor){  80,  80,  90, 255 };

        gui_fill_rect(gui, r, fill);

        GuiColor tc = (i == cursor)
            ? (GuiColor){  20,  20,  24, 255 }
            : (GuiColor){ 230, 230, 230, 255 };

        int tw = gui_text_width(gui, menu[i]);
        int th = gui_text_height(gui);
        int tx = r.x + (r.w - tw) / 2;
        int ty = r.y + (r.h - th) / 2;

        gui_draw_text(gui, tx, ty, tc, menu[i]);
    }

    // 여기서 present 하지 않음 (main에서 gui_end_frame이 처리)
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

static void handle_input(int key)
{
    if (key == IK_UP)   cursor = (cursor - 1 + MENU_COUNT) % MENU_COUNT;
    if (key == IK_DOWN) cursor = (cursor + 1) % MENU_COUNT;

    if (key == IK_CONFIRM) do_select();
    if (key == IK_CANCEL)  scene_request_quit();
}

static void exit_(void)
{
}

Scene g_scene_title = {
    .name = "title",
    .enter = enter,
    .update = update,
    .render = render,
    .handle_input = handle_input,
    .exit = exit_,
};