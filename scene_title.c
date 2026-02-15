#include <ncurses.h>
#include <string.h>
#include "scene.h"
#include "scene_manager.h"

// 다른 씬의 "Scene 객체"를 참조하기 위해 extern
extern Scene g_scene_game;

static void enter(void) { /* 필요 시 */ }
static void exit_(void) { /* 필요 시 */ }

static void render(void) {
    clear();
    int h, w;
    getmaxyx(stdscr, h, w);

    const char* title = "TETRIS (Terminal)";
    mvprintw(h/2 - 4, (w - (int)strlen(title))/2, "%s", title);

    mvprintw(h/2 - 1, (w - 20)/2, "1) START");
    mvprintw(h/2 + 0, (w - 20)/2, "Q) QUIT");
    refresh();
}

static void handle_input(int ch) {
    if (ch == '1') {
        scene_set(&g_scene_game);
    }
    // q 처리는 main에서 전역 처리해도 되고, 여기서 종료 플래그를 세워도 됩니다.
}

static void update(int dt_ms) { (void)dt_ms; }

Scene g_scene_title = {
    .name = "title",
    .enter = enter,
    .exit = exit_,
    .update = update,
    .render = render,
    .handle_input = handle_input
};