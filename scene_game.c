#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncursesw/ncurses.h>

#include "config.h"
#include "tetris.h"
#include "tetris_data.h"
#include "draw.h"
#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "settings.h"

#define LINESCORE1 10
#define LINESCORE2 30
#define LINESCORE3 50
#define LINESCORE4 80

static inline bool wasd_on(void) { return g_settings.wasd == WASD_ON; }
static inline bool key_left(int ch)  { return ch == KEY_LEFT  || (wasd_on() && (ch=='a'||ch=='A')); }
static inline bool key_right(int ch) { return ch == KEY_RIGHT || (wasd_on() && (ch=='d'||ch=='D')); }
static inline bool key_down(int ch)  { return ch == KEY_DOWN  || (wasd_on() && (ch=='s'||ch=='S')); }
static inline bool key_rot(int ch)   { return ch == KEY_UP    || (wasd_on() && (ch=='w'||ch=='W')); }

//애니메이션용 전역및 상태변수///////////////////////////////////////
typedef enum { GAME_PLAY = 0, GAME_LINECLEAR } GameMode;
static GameMode g_mode;
typedef struct {
    int rows[4];
    int count;
    int frame;
} LineClearAnim;
static LineClearAnim g_lc;
// BOARD_WIDTH=10이면 7프레임(0~6)
#define LINECLEAR_FRAMES ((BOARD_WIDTH / 2) + 2)
//////////////////////////////////////////////////////////////////


//전역 변수
static int drop_interval_ticks;
static int y, x, type, rot, timer;
// 점수/목표
static int score;
static int goal;
// 무한모드용(스테이지 0)
static int lines_total;
static int level;
// HOLD
static int hold_type;            // -1 이면 비어있음
static bool hold_used_this_turn; // 한 미노당 1회만 홀드 가능

//퍼즈용 변수////////////////////////////////////////////////////////////////
static bool is_paused = false;
static int  pause_cursor = 0; // 0: Resume, 1: Restart, 2: Main Menu
//////////////////////////////////////////////////////////////////////////

//넥스트
#define NEXT_COUNT 5
static int next_queue[NEXT_COUNT];
// ===== randomizer (PURE / 7-BAG) =====//////////////////////////////////////////////////////
static int bag[BLOCK_KIND];
static int bag_pos = BLOCK_KIND;

static void bag_shuffle(void) {
    for (int i = 0; i < BLOCK_KIND; i++) bag[i] = i;
    for (int i = BLOCK_KIND - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = bag[i]; bag[i] = bag[j]; bag[j] = tmp;
    }
    bag_pos = 0;
}

static void bag_reset(void) {
     bag_pos = BLOCK_KIND; 
}

static int bag_draw(void) {
    if (bag_pos >= BLOCK_KIND) bag_shuffle();
    return bag[bag_pos++];
}

static int random_draw(void) {
    if (g_settings.randomizer == RNG_7BAG) return bag_draw();
    return rand() % BLOCK_KIND;
}

static void refill_next_queue(void) {
    if (g_settings.randomizer == RNG_7BAG) bag_reset();
    for (int i = 0; i < NEXT_COUNT; i++) next_queue[i] = random_draw();
}

static int pop_next(void) {
    int t = next_queue[0];
    for (int i = 0; i < NEXT_COUNT - 1; i++) next_queue[i] = next_queue[i + 1];
    next_queue[NEXT_COUNT - 1] = random_draw();
    return t;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////


//저장 데이터
int g_last_stage = 1;
int g_last_score = 0;
int g_last_blocks_used = 0;
static int blocks_used = 0;

static void reset_active_pos(void) {
    y = 0;
    x = BOARD_WIDTH / 2 - 2;
    rot = 0;
}

static void spawn(void) {
    reset_active_pos();
    type = pop_next();
}

static int score_for_lines(int lines) {
    switch (lines) {
        case 1: return LINESCORE1;
        case 2: return LINESCORE2;
        case 3: return LINESCORE3;
        case 4: return LINESCORE4;
        default: return 0;
    }
}

static int stage_to_interval(int stage) {
    // 값이 작을수록 더 빨리 떨어짐(틱 간격이 짧음)
    switch (stage) {
        case 1: return 14; // 느림
        case 2: return 11;
        case 3: return 9;
        case 4: return 7;
        case 5: return 6;  // 매우 빠름
        case 6: return 5; // 느림
        case 7: return 4;
        case 8: return 3;
        case 9: return 2;
        case 10: return 1;  // 매우 빠름
        default: return 9;
    }
}

static inline bool is_infinite_mode(void) {
    return g_selected_stage == 0;
}

static void infinite_recalc_speed(void) {
    // 10라인마다 레벨업(최대 10). 레벨은 낙하속도에만 영향.
    int lv = 1 + (lines_total / 10);
    if (lv > 10) lv = 10;
    level = lv;
    drop_interval_ticks = stage_to_interval(level);
}

static void enter(void) {
    drop_interval_ticks = stage_to_interval(g_selected_stage);
    g_mode = GAME_PLAY;
    g_lc.count = 0;
    g_lc.frame = 0;

    // 재시작 시 보드 잔상/이전 판 상태 제거
    memset(board, 0, sizeof(board));
    timer = 0;
    // 7-bag은 판 시작 시점에서 리셋 (재시작도 동일)
    bag_reset();
    refill_next_queue();
    spawn();

    is_paused = false;
    pause_cursor = 0;

    hold_type = -1;
    hold_used_this_turn = false;

    score = 0;
    if (is_infinite_mode()) {
        goal = -1;        // 무한모드: goal 사용 안 함
        lines_total = 0;
        level = 1;
    } else {
        goal = stage_goal_score(g_selected_stage);
        lines_total = 0;
        level = 0;
    }
    
    blocks_used = 0;
}

static void set_last_result(void) {
    g_last_stage = g_selected_stage;
    g_last_score = score;
    g_last_blocks_used = blocks_used;
}

static void spawn_and_check_gameover(void) {
    spawn();
    timer = 0;
    if (check_collision(y, x, type, rot)) {
        set_last_result();
        scene_set(&g_scene_gameover);
    }
}

static bool begin_lineclear_anim_if_needed(void) {
    int rows[4];
    int cnt = tetris_find_full_lines(rows);
    if (cnt <= 0) return false;

    g_lc.count = cnt;
    for (int i = 0; i < cnt; i++) g_lc.rows[i] = rows[i];
    g_lc.frame = 0;
    g_mode = GAME_LINECLEAR;
    timer = 0;
    return true;
}

static void render(void) {
    erase();
    draw_board(board);

    if (g_mode == GAME_LINECLEAR) {
        draw_line_clear_anim(g_lc.rows, g_lc.count, g_lc.frame);
    }

    if (g_mode == GAME_PLAY) {
        if (g_settings.ghost) {
            int gy = y;
            while (!check_collision(gy + 1, x, type, rot)) gy++;
            if (gy != y) draw_ghost_block(gy, x, type, rot);
        }
        draw_block(y, x, type, rot);
    }

    // 보드 오른쪽 시작 x
    int ui_left = (BOARD_WIDTH + 4) * 2;
    //===== NEXT (5개 세로) =====
    mvaddstr(1, ui_left, "NEXT");
    for (int i = 0; i < NEXT_COUNT; i++) {
        int top = 3 + i * BLOCK_SIZE;     // 3~22 사용
        draw_mino_preview(top, ui_left, next_queue[i], 0);
    }
    //===== 점수/스테이지 UI는 NEXT보다 더 오른쪽으로 =====
    int info_x = ui_left + (BLOCK_SIZE * 2) + 6; // NEXT 폭(4칸*2=8) + 여백
    if (is_infinite_mode()) {
        mvprintw(2, info_x, "MODE  : INFINITE");
        mvprintw(3, info_x, "SCORE : %d", score);
        mvprintw(4, info_x, "LEVEL : %d", level);
        mvprintw(5, info_x, "LINES : %d", lines_total);
    } else {
        mvprintw(2, info_x, "STAGE : %d", g_selected_stage);
        mvprintw(3, info_x, "SCORE : %d", score);
        mvprintw(4, info_x, "GOAL  : %d", goal);
    }
    // ===== HOLD =====
    if (g_settings.hold) {
        mvaddstr(6, info_x, "HOLD");
        if (hold_type >= 0) draw_mino_preview(7, info_x, hold_type, 0);
        else mvaddstr(7, info_x, "(empty)");
    } else {
        mvaddstr(6, info_x, "HOLD : OFF");
    }
    //퍼즈 기능 
    if (is_paused)
    {
        int h, w;
        getmaxyx(stdscr, h, w);
        int box_w = 30;
        int box_h = 8;
        int start_y = h / 2 - box_h / 2;
        int start_x = w / 2 - box_w / 2;
        // 박스
        for (int i = 0; i < box_h; i++)
            mvhline(start_y + i, start_x, ' ', box_w);
        mvprintw(start_y + 1, start_x + 10, "PAUSED");
        const char* menu[3] = {
            "Resume",
            "Restart",
            "Main Menu"
        };
        for (int i = 0; i < 3; i++)
        {
            if (i == pause_cursor)
                attron(A_REVERSE);
            mvprintw(start_y + 3 + i, start_x + 8, "%s", menu[i]);
            if (i == pause_cursor)
                attroff(A_REVERSE);
        }
    }

    refresh();
}

static void handle_input(int ch) {

    if (g_mode == GAME_LINECLEAR) return;
    if (is_paused)
    {
        switch (ch)
        {
        case KEY_UP:
        case 'w':
            pause_cursor = (pause_cursor + 2) % 3;
            break;
        case KEY_DOWN:
        case 's':
            pause_cursor = (pause_cursor + 1) % 3;
            break;
        case 10: // Enter
            if (pause_cursor == 0)
            {
                is_paused = false;   // Resume
            }
            else if (pause_cursor == 1)
            {
                scene_set(&g_scene_game); // Restart
            }
            else if (pause_cursor == 2)
            {
                scene_set(&g_scene_title); // Main Menu
            }
            break;

        case 27: // ESC
            is_paused = false;
            break;
        }
        return;
    }

    // ===== 일반 게임 입력 =====

    if (ch == 'p')
    {
        is_paused = true;
        pause_cursor = 0;
        return;
    }

    if (key_left(ch)  && !check_collision(y, x - 1, type, rot)) x--;
    if (key_right(ch) && !check_collision(y, x + 1, type, rot)) x++;
    if (key_down(ch)  && !check_collision(y + 1, x, type, rot)) y++;

    if (key_rot(ch)) {
        int next = (rot + 1) % 4;
        if (!check_collision(y, x, type, next)) rot = next;
    }

    if (ch == ' ') {
        while (!check_collision(y + 1, x, type, rot)) y++;
        blocks_used++;
        freeze_block(y, x, type, rot);
        hold_used_this_turn = false;
        g_lc.count = tetris_find_full_lines(g_lc.rows);
        if (begin_lineclear_anim_if_needed()) {
            return; // 애니메이션 모드로 넘어감 (점수/클리어 판정은 애니 종료 시점에서)
        }
        spawn_and_check_gameover();
    }

    if(g_settings.hold) {
    // HOLD (한 미노당 1회)
        if ((ch == 'c' || ch == 'C') && !hold_used_this_turn) {
            hold_used_this_turn = true;

            if (hold_type < 0) {
                // 첫 홀드: 현재 미노를 저장하고 next에서 새로 꺼냄
                hold_type = type;
                type = pop_next();
            } else {
                // 스왑
                int tmp = type;
                type = hold_type;
                hold_type = tmp;
            }

            reset_active_pos();
            if (check_collision(y, x, type, rot)) {
                g_last_stage = g_selected_stage;
                g_last_score = score;
                g_last_blocks_used = blocks_used;
                scene_set(&g_scene_gameover);
            }
        }
    }
}

static void update(int dt_ms) {
    (void)dt_ms;

    if (g_mode == GAME_LINECLEAR) {
        g_lc.frame++;
        if (g_lc.frame >= LINECLEAR_FRAMES) {
            tetris_remove_lines(g_lc.rows, g_lc.count);

            score += score_for_lines(g_lc.count);
            if (is_infinite_mode()) {
                lines_total += g_lc.count;
                infinite_recalc_speed();
            } else if (score >= goal) {
                stage_mark_cleared(g_selected_stage);
                set_last_result();
                scene_set(&g_scene_stage_clear);
                g_mode = GAME_PLAY;
                return;
            }

            g_mode = GAME_PLAY;
            spawn_and_check_gameover();
        }
        return;
    }

    if (is_paused)
    return;

    timer++;

    if (timer > drop_interval_ticks) {
        if (!check_collision(y + 1, x, type, rot)) {
            y++;
        } else {
            blocks_used++;
            freeze_block(y, x, type, rot);
            hold_used_this_turn = false;
            if (begin_lineclear_anim_if_needed()) {
                timer = 0;
                return;
            }
           

            spawn_and_check_gameover();
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
