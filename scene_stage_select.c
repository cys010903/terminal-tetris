#include <ncurses.h>
#include <string.h>

#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "config.h"

// 다른 씬
extern Scene g_scene_game;
extern Scene g_scene_title;

// 기본 스테이지(프로그램 시작 시 1)
int g_selected_stage = 1;

static int cursor_stage = 1; // 현재 커서(선택 후보)

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static const char* stage_desc(int stage) {
    switch (stage) {
        case 1: return "stage1";
        case 2: return "stage2";
        case 3: return "stage3";
        case 4: return "stage4";
        case 5: return "stage5";
        case 6: return "stage6";
        case 7: return "stage7";
        case 8: return "stage8";
        case 9: return "stage9";
        case 10: return "stage10";
        default: return "";
    }
}

static void enter(void) {
    cursor_stage = clampi(g_selected_stage, 1, NUMBER_OF_STAGES);
}

static void render(void) {
    erase();

    int h, w;
    getmaxyx(stdscr, h, w);

    const char* title = "STAGE SELECT";
    mvprintw(h/2 - 6, (w - (int)strlen(title))/2, "%s", title);

    const char* help1 = "UP/DOWN: Move   ENTER/SPACE: Confirm";
    const char* help2 = "B/ESC: Back to Title";
    mvprintw(h/2 - 4, (w - (int)strlen(help1))/2, "%s", help1);
    mvprintw(h/2 - 3, (w - (int)strlen(help2))/2, "%s", help2);

    int base_y = h/2 - 1;
    for (int s = 1; s <= NUMBER_OF_STAGES; s++) {
        char line[64];
        snprintf(line, sizeof(line), "Stage %d  (%s)", s, stage_desc(s));

        int x0 = (w - (int)strlen(line))/2;
        if (s == cursor_stage) {
            attron(A_REVERSE);
            mvprintw(base_y + (s-1), x0, "%s", line);
            attroff(A_REVERSE);
        } else {
            mvprintw(base_y + (s-1), x0, "%s", line);
        }

        if (s == g_selected_stage) {
            mvprintw(base_y + (s-1), x0 - 4, "*");
        }
    }

    refresh();
}

static void handle_input(int ch) {
    if (ch == KEY_UP) {
        cursor_stage = clampi(cursor_stage - 1, 1, NUMBER_OF_STAGES);
        return;
    }
    if (ch == KEY_DOWN) {
        cursor_stage = clampi(cursor_stage + 1, 1, NUMBER_OF_STAGES);
        return;
    }

    // 숫자키로 스테이지 선택 (1~9)
    if (ch >= '1' && ch <= '9') {
        int s = ch - '0';
        if (s <= NUMBER_OF_STAGES) cursor_stage = s;
        return;
    }
    // 10 스테이지가 있을 때 '0'을 10으로 처리(선택사항)
    if (ch == '0' && NUMBER_OF_STAGES >= 10) {
        cursor_stage = 10;
        return;
    }

    if (ch == '\n' || ch == KEY_ENTER || ch == ' ') {
        g_selected_stage = clampi(cursor_stage, 1, NUMBER_OF_STAGES);
        scene_set(&g_scene_game);
        return;
    }

    if (ch == 'b' || ch == 'B' || ch == 27 /*ESC*/) {
        scene_set(&g_scene_title);
        return;
    }
}

static void update(int dt_ms) { (void)dt_ms; }

Scene g_scene_stage_select = {
    .name = "stage_select",
    .enter = enter,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input,
};
