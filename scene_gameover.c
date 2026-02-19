#include <ncurses.h>
#include "scene.h"
#include "scene_manager.h"

extern Scene g_scene_game;
extern Scene g_scene_title;

static void render(void) {
    erase();
    mvprintw(10, 10, "GAME OVER");
    mvprintw(12, 10, "R) Restart");
    mvprintw(13, 10, "T) Title");
    mvprintw(14, 10, "Q) Quit");
    refresh();
}

static void handle_input(int ch) {
    if (ch == 'r' || ch == 'R') scene_set(&g_scene_game);
    if (ch == 't' || ch == 'T') scene_set(&g_scene_title);
}

static void update(int dt_ms) { (void)dt_ms; }

Scene g_scene_gameover = {
    .name = "gameover",
    .enter = 0,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input
};