#include <ncurses.h>
#include <stdlib.h>
#include "scene.h"
#include "scene_manager.h"
#include "common.h"

extern Scene g_scene_gameover;

static int y, x, type, rot, timer;

static void spawn(void) {
    y = 0;
    x = BOARD_WIDTH / 2 - 2;
    type = rand() % BLOCK_KIND;
    rot = 0;
}

static void enter(void) {
    timer = 0;
    spawn();
}

static void render(void) {
    clear();
    draw_board(board);
    draw_block(y, x, type, rot);
    refresh();
}

static void handle_input(int ch) {
    if (ch == KEY_LEFT && !check_collision(y, x - 1, type, rot)) x--;
    if (ch == KEY_RIGHT && !check_collision(y, x + 1, type, rot)) x++;
    if (ch == KEY_DOWN && !check_collision(y + 1, x, type, rot)) y++;

    if (ch == KEY_UP) {
        int next = (rot + 1) % 4;
        if (!check_collision(y, x, type, next)) rot = next;
    }

    if (ch == ' ') {
        while (!check_collision(y + 1, x, type, rot)) y++;
        freeze_block(y, x, type, rot);
        clear_lines();
        spawn();
        if (check_collision(y, x, type, rot)) {
            scene_set(&g_scene_gameover);
        }
    }
}

static void update(int dt_ms) {
    (void)dt_ms;
    timer++;

    if (timer > 5) {
        if (!check_collision(y + 1, x, type, rot)) {
            y++;
        } else {
            freeze_block(y, x, type, rot);
            clear_lines();
            spawn();
            if (check_collision(y, x, type, rot)) {
                scene_set(&g_scene_gameover);
            }
        }
        timer = 0;
    }
}

Scene g_scene_game = {
    .name = "game",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input
};
