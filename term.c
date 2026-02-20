#include "term.h"
#include "config.h"
#include <string.h> // strlen

int check_terminal_size(void) {
    int h, w;
    getmaxyx(stdscr, h, w);
    return (h >= MIN_TERM_HEIGHT && w >= MIN_TERM_WIDTH);
}

static void mvaddstr_safe(int y, int x, const char* s) {
    int h, w;
    getmaxyx(stdscr, h, w);

    if (y < 0 || y >= h) return;
    if (x < 0) x = 0;
    if (x >= w) return;

    // 화면 밖으로 넘치는 문자열은 잘라서 출력
    mvaddnstr(y, x, s, w - x - 1);
}

void render_resize_prompt(void) {
    int h, w;
    getmaxyx(stdscr, h, w);

    char line2[128];
    char line3[128];
    const char* line1 = "TERMINAL TOO SMALL";
    snprintf(line2, sizeof line2, "Required: >= %d rows x %d cols", MIN_TERM_HEIGHT, MIN_TERM_WIDTH);
    snprintf(line3, sizeof line3, "Current :    %d rows x %d cols", h, w);
    const char* line4 = "Resize the window. (Press Q to quit)";

    erase();

    int y = h / 2 - 2;
    mvaddstr_safe(y + 0, (w - (int)strlen(line1)) / 2, line1);
    mvaddstr_safe(y + 1, (w - (int)strlen(line2)) / 2, line2);
    mvaddstr_safe(y + 2, (w - (int)strlen(line3)) / 2, line3);
    mvaddstr_safe(y + 4, (w - (int)strlen(line4)) / 2, line4);

    refresh();
}