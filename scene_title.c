#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <ncurses.h>
#include <string.h>
#include <stdlib.h> 
#include <wchar.h>
#include <locale.h>
#include "scene.h"
#include "scene_manager.h"
#include "title_ascii.h"

// 다른 씬의 "Scene 객체"를 참조하기 위해 extern
extern Scene g_scene_game;
extern Scene g_scene_stage_select;
extern Scene g_scene_records;


static void enter(void) { /* 필요 시 */ }
static void exit_(void) { /* 필요 시 */ }

static int cursor = 0;
static const int MENU_COUNT = 3;

static int str_col_width(const char* s) {
    if (!s) return 0;

    // 멀티바이트(UTF-8) -> 와이드 문자로 변환
    wchar_t wbuf[1024];
    size_t n = mbstowcs(wbuf, s, 1023);
    if (n == (size_t)-1) {
        // 변환 실패 시 안전하게 strlen로 fallback
        return (int)strlen(s);
    }
    wbuf[n] = L'\0';

    int w = wcswidth(wbuf, (int)n);
    if (w < 0) {
        // 폭 계산 실패 시 fallback
        return (int)strlen(s);
    }
    return w;
}

static int max_line_width(const char* const* lines, int count) {
    int maxw = 0;
    for (int i = 0; i < count; i++) {
        int w = str_col_width(lines[i]);
        if (w > maxw) maxw = w;
    }
    return maxw;
}

static void render(void) {
    erase();

    int h, w;
    getmaxyx(stdscr, h, w);

    // ===== 유니코드 기준 폭 계산 =====
    int maxw = max_line_width(TITLE_ART, TITLE_ART_LINES);

    int start_y = 2;
    int start_x = (w - maxw) / 2;
    if (start_x < 0) start_x = 0;

    if (start_y + TITLE_ART_LINES + 6 > h) {
        start_y = 0;
    }

    // ===== 로고 출력 =====
    for (int i = 0; i < TITLE_ART_LINES; i++) {
        mvaddstr(start_y + i, start_x, TITLE_ART[i]);
    }

    // ===== 메뉴 =====
    int menu_y = start_y + TITLE_ART_LINES + 2;

    const char* menus[] = {
        "START",
        "RECORDS",
        "QUIT"
    };
    int base_y = h/2 -1;
    
    for (int i = 0; i < MENU_COUNT; i++) {
        const char* line = menus[i];
        int x0 = (w - (int)strlen(line)) / 2;
        if (x0 < 0) x0 = 0;

        if (i == cursor) {
            attron(A_REVERSE);
            mvprintw(base_y + i, x0, "%s", line);
            attroff(A_REVERSE);
        } else {
            mvprintw(base_y + i, x0, "%s", line);
        }
    }

    menu_y += 3;
    const char* hint = "[←/→ Move] [↓ Soft] [↑ Rotate] [Space Hard] [C Hold]";
    mvaddstr(menu_y, (w - (int)strlen(hint)) / 2, hint);

    refresh();
}


static void handle_input(int ch) {
    switch (ch) {
    case KEY_UP:  cursor = (cursor - 1 + MENU_COUNT) % MENU_COUNT; break;
    case KEY_DOWN: cursor = (cursor + 1) % MENU_COUNT; break;
    case '\n':
    case KEY_ENTER:
        if (cursor == 0) scene_set(&g_scene_stage_select);
        else if(cursor == 1) scene_set(&g_scene_records);
        else { endwin(); exit(0); }
        break;
    case '1': cursor = 0; scene_set(&g_scene_stage_select); break;
    case 'q':
    case 'Q': endwin(); exit(0); break;
    }
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
