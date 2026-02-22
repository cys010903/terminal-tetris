#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <stddef.h>
#include <stdbool.h>
#include <ncurses.h>

#include "config.h"
#include "scene.h"
#include "records.h"
#include "draw.h"
#include "scene_manager.h"
#include "settings.h"

int main(void) {
    srand((unsigned int)time(NULL));
    setlocale(LC_ALL, "");

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);          // getch()가 최대 100ms 대기 (입력 없어도 루프 진행)

    init_colors();
    settings_load();
    records_init();

    // 타이틀 씬부터 시작
    scene_set(&g_scene_title);

    // 100ms 틱 기반으로 흘러가도록 dt를 고정
    const int dt_ms = 100;

    while (1) {
        int ch = getch();

        // 전역 종료 (어떤 씬에서든 q로 종료)
        if (ch == 'q' || ch == 'Q') break;

        

        if (ch != ERR) {
            scene_input(ch);
        }

        scene_update(dt_ms);
        scene_render();
    }

    // 현재 씬 exit 호출까지 정리
    scene_set(NULL);

    endwin();
    return 0;
}
