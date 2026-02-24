// main_sdl.c
#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "gui.h"
#include "scene_manager.h"
#include "term_compat.h"
#include "common.h"
#include "input_sdl.h"
#include "settings.h"
#include "records.h"
#include "stage.h"
#include "app_context.h"

static int map_key(SDL_Keycode sym)
{
    return ik_from_sdl_key(sym);
}

int main(void)
{
    AppContext* ctx = app_ctx();

    srand((unsigned int)time(NULL));

    const int W = 1280, H = 720;
    Gui* gui = gui_create(W, H, "Tetris (SDL2)");
    if (!gui) {
        fprintf(stderr, "gui_init failed\n");
        return 1;
    }

    if (!gui_text_init(gui, NULL, 18)) {
        fprintf(stderr, "gui_text_init failed\n");
        gui_destroy(gui);
        return 1;
    }

    gui_set_bg(gui, (GuiColor){ 35, 40, 55, 255 });
    term_bind_gui(gui);

    settings_load(&ctx->settings);
    records_init();
    stage_clear_load();

    scene_set(&g_scene_title);

    const uint32_t target_frame_ms = 16;
    uint32_t last = gui_ticks_ms();

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) goto quit;

            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                int k = map_key(e.key.keysym.sym);
                if (k != IK_NONE) scene_input(k);
            }
        }

        if (scene_should_quit()) goto quit;

        uint32_t now = gui_ticks_ms();
        uint32_t dt = now - last;
        last = now;

        scene_update((int)dt);

        gui_begin_frame(gui);
        scene_render();
        gui_end_frame(gui);

        uint32_t used = gui_ticks_ms() - now;
        if (used < target_frame_ms) gui_sleep_ms(target_frame_ms - used);
    }

quit:
    records_flush();
    gui_destroy(gui);
    return 0;
}