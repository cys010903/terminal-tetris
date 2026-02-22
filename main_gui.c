// main_gui.c
#include "gui.h"
#include <stdio.h>

int main(void)
{
    Gui* gui = NULL;

    // 원하는 고정 창 크기
    const int W = 900;
    const int H = 600;

    if (!gui_init(&gui, "Tetris GUI (SDL2)", W, H)) {
        fprintf(stderr, "gui_init failed\n");
        return 1;
    }

    // 목표 2) 배경화면 색 구현: 여기만 바꾸면 됨
    gui_set_bg(gui, (GuiColor){ 35, 40, 55, 255 });

    const uint32_t target_frame_ms = 16; // ~60fps
    uint32_t last = gui_ticks_ms();

    while (1) {
        GuiInput in;
        gui_poll_input(gui, &in);
        if (in.quit) break;

        uint32_t now = gui_ticks_ms();
        uint32_t dt = now - last;
        if (dt < target_frame_ms) gui_sleep_ms(target_frame_ms - dt);
        last = gui_ticks_ms();

        gui_begin_frame(gui);
        // (지금 목표 범위에서는 여기서 끝)
        gui_end_frame(gui);
    }

    gui_shutdown(gui);
    return 0;
}