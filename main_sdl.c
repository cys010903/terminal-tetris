// main_sdl.c
#include "gui.h"
#include "scene_manager.h"
#include "term_compat.h"
#include "common.h"

#include <SDL2/SDL.h>
#include <ncurses.h> 
#include <stdio.h>

static int map_key(int sdl_sym)
{
    switch (sdl_sym) {
    case SDLK_UP:    return KEY_UP;
    case SDLK_DOWN:  return KEY_DOWN;
    case SDLK_LEFT:  return KEY_LEFT;
    case SDLK_RIGHT: return KEY_RIGHT;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        return '\n';
    default:
        break;
    }

    // 알파벳/스페이스 등은 기존 scene 코드가 char 비교를 하므로 그대로
    if (sdl_sym >= 32 && sdl_sym <= 126) return sdl_sym;
    return 0;
}

int main(void)
{
    Gui gui;

    const int W = 1000;
    const int H = 700;

    if (!gui_init(&gui, W, H, "Tetris (SDL2)")) {
        fprintf(stderr, "gui_init failed\n");
        return 1;
    }

    // 텍스트(유니코드 포함) 활성화
    if (!gui_text_init(&gui, NULL, 18)) {
        fprintf(stderr, "gui_text_init failed (SDL_ttf / font missing)\n");
        gui_shutdown(&gui);
        return 1;
    }

    gui_set_bg(&gui, (GuiColor){ 35, 40, 55, 255 });

    // ncurses 호환 레이어에 gui 바인딩
    term_bind_gui(&gui);

    // 첫 씬
    scene_set(&g_scene_title);

    const uint32_t target_frame_ms = 16;
    uint32_t last = gui_ticks_ms();

    while (1) {
        GuiInput in;
        gui_poll_input(&gui, &in);
        if (in.quit) break;

        if (in.last_key) {
            int ch = map_key(in.last_key);
            if (ch) scene_input(ch);
        }

        uint32_t now = gui_ticks_ms();
        uint32_t dt = now - last;
        last = now;

        scene_update((int)dt);

        gui_begin_frame(&gui);
        scene_render();
        gui_end_frame(&gui);

        // 프레임 캡
        uint32_t used = gui_ticks_ms() - now;
        if (used < target_frame_ms) gui_sleep_ms(target_frame_ms - used);
    }

    gui_shutdown(&gui);
    return 0;
}