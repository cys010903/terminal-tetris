#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "stage.h"
#include "scene.h"
#include "scene_manager.h"
#include "common.h"


//씬 데이터
extern Scene g_scene_gameover;
extern Scene g_scene_stage_clear;
//전역 변수
static int drop_interval_ticks;
static int y, x, type, rot, timer;
// 점수/목표
static int score;
static int goal;
// HOLD
static int hold_type;            // -1 이면 비어있음
static bool hold_used_this_turn; // 한 미노당 1회만 홀드 가능

#define NEXT_COUNT 5
static int next_queue[NEXT_COUNT];

static void refill_next_queue(void) {
    for (int i = 0; i < NEXT_COUNT; i++) {
        next_queue[i] = rand() % BLOCK_KIND;
    }
}

static int pop_next(void) {
    int t = next_queue[0];
    for (int i = 0; i < NEXT_COUNT - 1; i++) {
        next_queue[i] = next_queue[i + 1];
    }
    next_queue[NEXT_COUNT - 1] = rand() % BLOCK_KIND;
    return t;
}

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
        case 1: return 100;
        case 2: return 300;
        case 3: return 500;
        case 4: return 800;
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

static void enter(void) {
    drop_interval_ticks = stage_to_interval(g_selected_stage);

    // 재시작 시 보드 잔상/이전 판 상태 제거
    memset(board, 0, sizeof(board));
    timer = 0;
    refill_next_queue();
    spawn();

    hold_type = -1;
    hold_used_this_turn = false;

    score = 0;
    goal = stage_goal_score(g_selected_stage);
}

static void render(void) {
    erase();
    draw_board(board);
    // ===== GHOST (착지 예상 위치) =====
    int gy = y;
    while (!check_collision(gy + 1, x, type, rot)) {
        gy++;
    }
    if (gy != y) {
        draw_ghost_block(gy, x, type, rot);
    }   

    // 실제 조작 중인 블록은 고스트 위에 그리기
    draw_block(y, x, type, rot);

    // 보드 오른쪽 시작 x
    int ui_left = (BOARD_WIDTH + 4) * 2;

    // ===== NEXT (5개 세로) =====
    mvaddstr(1, ui_left, "NEXT");
    for (int i = 0; i < NEXT_COUNT; i++) {
        int top = 3 + i * BLOCK_SIZE;     // 3~22 사용
        draw_mino_preview(top, ui_left, next_queue[i], 0);
    }

    // ===== 점수/스테이지 UI는 NEXT보다 더 오른쪽으로 =====
    int info_x = ui_left + (BLOCK_SIZE * 2) + 6; // NEXT 폭(4칸*2=8) + 여백
    mvprintw(2, info_x, "STAGE : %d", g_selected_stage);
    mvprintw(3, info_x, "SCORE : %d", score);
    mvprintw(4, info_x, "GOAL  : %d", goal);

    // ===== HOLD =====
    mvaddstr(6, info_x, "HOLD");
    if (hold_type >= 0) {
        draw_mino_preview(7, info_x, hold_type, 0);
    } else {
        mvaddstr(7, info_x, "(empty)");
    }
    mvaddstr(12, info_x, "C : HOLD");

    refresh();
}

static void handle_input(int ch) {
    if (ch == KEY_LEFT && !check_collision(y, x - 1, type, rot)) x--;
    if (ch == KEY_RIGHT && !check_collision(y, x + 1, type, rot)) x++;
    if (ch == KEY_DOWN && !check_collision(y + 1, x, type, rot)) y++;

    if (ch == KEY_UP) {
        int next = (rot + 1) % 4;
        if (!check_collision(y, x, type, next)) rot = next;
    }

    if (ch == ' ') {
        while (!check_collision(y + 1, x, type, rot)) y++;
        freeze_block(y, x, type, rot);
        hold_used_this_turn = false;
        int cleared = clear_lines();
        if (cleared > 0) {
            score += score_for_lines(cleared) * g_selected_stage; // 스테이지 가중치(원치 않으면 *stage 제거)
            if (score >= goal) {
                scene_set(&g_scene_stage_clear);
                return;
            }
        }
        spawn();
        if (check_collision(y, x, type, rot)) {
            scene_set(&g_scene_gameover);
        }
    }

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
            scene_set(&g_scene_gameover);
        }
    }
}

static void update(int dt_ms) {
    (void)dt_ms;
    timer++;

    if (timer > drop_interval_ticks) {
        if (!check_collision(y + 1, x, type, rot)) {
            y++;
        } else {
            freeze_block(y, x, type, rot);
            hold_used_this_turn = false;
            int cleared = clear_lines();
            if (cleared > 0) {
                score += score_for_lines(cleared);
                if (score >= goal) {
                    scene_set(&g_scene_stage_clear);
                    timer = 0;
                    return;
                }
            }

            spawn();
            if (check_collision(y, x, type, rot)) {
                scene_set(&g_scene_gameover);
            }
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
