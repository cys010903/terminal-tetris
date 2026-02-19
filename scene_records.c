#include <ncurses.h>
#include "scene.h"
#include "scene_manager.h"
#include "records.h"

extern Scene g_scene_title;

static long page = 0;                 // 0=최신 페이지(링버퍼), 1..=과거 페이지(파일)
static const int page_size = RECORD_MAX; // 10개 고정(링버퍼와 동일)

static long calc_max_page(long total){
    if (total <= 0) return 0;
    return (total - 1) / page_size;   // 0-indexed
}

static void clamp_page(long total){
    long max_page = calc_max_page(total);
    if (page < 0) page = 0;
    if (page > max_page) page = max_page;
}

static void render_list_header(int y){
    mvprintw(y,   2, "No  Stage   Score   BlocksUsed");
    mvprintw(y+1, 2, "--------------------------------");
}

static void render(void){
    clear();

    int h, w;
    getmaxyx(stdscr, h, w);
    (void)w;

    long total = records_log_count();          // 파일에 쌓인 총 판 수
    long max_page = calc_max_page(total);
    clamp_page(total);

    // ===== 상단 표시 (요구사항 2) =====
    // page=0이 최신, 화면 표시는 1부터
    mvprintw(1, 2, "RECORDS   Total %ld   Page %ld / %ld",
             total, page + 1, max_page + 1);
    mvprintw(2, 2, "<-/a : Newer   ->/d : Older   ESC/B : Back");

    int y0 = 4;
    render_list_header(y0);

    int y = y0 + 2;

    // ===== 요구사항 3) 최신 페이지(page=0)만 링버퍼 사용 =====
    if (page == 0) {
        int cnt = records_count(); // 링버퍼에 들어있는 개수(최대 10)
        if (cnt <= 0) {
            mvaddstr(y, 2, "(no records yet)");
            refresh();
            return;
        }

        for (int i = 0; i < cnt; i++) {
            const RecordEntry* e = records_get_latest(i); // i=0이 가장 최신
            if (!e) break;

            long global_no = (page * page_size) + i + 1; // 최신 기준 1부터
            mvprintw(y + i, 2, "%2ld  %5d  %6d  %10d",
                     global_no, e->stage, e->score, e->blocks_used);

            if (y + i >= h - 2) break;
        }
    }
    // ===== 과거 페이지(page>=1)는 파일에서 읽기 =====
    else {
        RecordEntry tmp[RECORD_MAX];
        int got = records_log_read_latest_range(page * page_size, page_size, tmp);
        if (got <= 0) {
            mvaddstr(y, 2, "(no more records)");
            refresh();
            return;
        }

        for (int i = 0; i < got; i++) {
            long global_no = (page * page_size) + i + 1;
            mvprintw(y + i, 2, "%2ld  %5d  %6d  %10d",
                     global_no, tmp[i].stage, tmp[i].score, tmp[i].blocks_used);

            if (y + i >= h - 2) break;
        }
    }

    refresh();
}

static void handle_input(int ch){
    // older page (오래된 기록으로)
    if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
        page++;
    }
    // newer page (최신 기록으로)
    else if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
        if (page > 0) page--;
    }
    // back
    else if (ch == 27 || ch == 'b' || ch == 'B') {
        scene_set(&g_scene_title);
    }
}

static void update(int dt_ms){ (void)dt_ms; }

Scene g_scene_records = {
    .name = "records",
    .enter = 0,
    .exit = 0,
    .update = update,
    .render = render,
    .handle_input = handle_input
};

