#include <ncurses.h>
#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "config.h"
#include "records.h"

extern int g_last_stage, g_last_score, g_last_blocks_used;

static void enter(void) {
	records_push(g_last_stage, g_last_score, g_last_blocks_used);
}


static void render(void) {
    erase();
    mvprintw(10, 10, "STAGE CLEAR!");
    mvprintw(12, 10, "N) Next Stage");
    mvprintw(13, 10, "S) Stage Select");
    mvprintw(14, 10, "T) Title");
    refresh();
}

static void handle_input(int ch) {
    if (ch == 'n' || ch == 'N') {
        if (g_selected_stage < NUMBER_OF_STAGES) g_selected_stage++;
        scene_set(&g_scene_game);
        return;
    }
    if (ch == 's' || ch == 'S') {
        scene_set(&g_scene_stage_select);
        return;
    }
    if (ch == 't' || ch == 'T') {
        scene_set(&g_scene_title);
        return;
    }
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
