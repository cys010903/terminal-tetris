#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "tetris.h"
#include "tetris_data.h"
#include "draw.h"
#include "draw_game.h"
#include "game_view.h"
#include "scene.h"
#include "scene_manager.h"
#include "stage.h"
#include "settings.h"
#include "term_compat.h"
#include "gui.h"
#include "input_keys.h"

#ifndef BUILD_SDL
#include <ncursesw/ncurses.h>
#endif

#define LINESCORE1 10
#define LINESCORE2 30
#define LINESCORE3 50
#define LINESCORE4 80

// ===== game mode / lineclear anim =====
typedef enum { GAME_PLAY = 0, GAME_LINECLEAR } GameMode;
static GameMode g_mode;

// ===== core state =====
// NOTE: SDL에서는 프레임이 빠르므로 "틱" 기반 로직을 전부 ms 기반으로 통일한다.
static int drop_interval_ms;   // 중력 간격(ms). 값이 작을수록 빠름
static int drop_acc_ms;        // 중력 누적(ms)
static int lc_acc_ms;          // 라인클리어 애니 누적(ms)
static int y, x, type, rot;

// 점수/목표
static int score;
static int goal;
// 무한모드용(스테이지 0)
static int lines_total;
static int level;

// ===== game function =====
// HOLD
static int hold_type;            // -1 이면 비어있음
static bool hold_used_this_turn; // 한 미노당 1회만 홀드 가능
// PAUSE
static int  pause_cursor = 0; // 0: Resume, 1: Restart, 2: Main Menu

typedef struct {
    int rows[4];
    int count;
    int frame;
} LineClearAnim;

static LineClearAnim g_lc;
#define LINECLEAR_FRAMES ((BOARD_WIDTH / 2) + 2)
// 라인클리어 애니 1프레임을 몇 ms로 볼지(속도 조절 포인트)
#define LINECLEAR_FRAME_MS (70)

// ===== SDL RENDER =====
typedef enum { OVERLAY_NONE, OVERLAY_PAUSE } Overlay;
static Overlay g_overlay = OVERLAY_NONE;

// ===== NEXT =====
#define NEXT_COUNT 5
static int next_queue[NEXT_COUNT];

// ===== randomizer (PURE / 7-BAG) =====
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

static void bag_reset(void) { bag_pos = BLOCK_KIND; }
static int bag_draw(void) { if (bag_pos >= BLOCK_KIND) bag_shuffle(); return bag[bag_pos++]; }

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

// ===== SAVE DATE =====
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

static int stage_to_gravity_ms(int stage)
{
    // 값이 작을수록 더 빨리 떨어짐(ms)
    // (기존 tick 기반 1~14는 SDL에서 초고속이므로 ms 커브로 교체)
    switch (stage) {
        case 1:  return 900;
        case 2:  return 780;
        case 3:  return 680;
        case 4:  return 590;
        case 5:  return 520;
        case 6:  return 460;
        case 7:  return 410;
        case 8:  return 360;
        case 9:  return 320;
        case 10: return 280;
        default: return 700;
    }
}

static inline bool is_infinite_mode(void) { return g_selected_stage == 0; }

static void infinite_recalc_speed(void) {
    // 10라인마다 레벨업(최대 10). 레벨은 낙하속도에만 영향.
    int lv = 1 + (lines_total / 10);
    if (lv > 10) lv = 10;
    level = lv;
    drop_interval_ms = stage_to_gravity_ms(level);
}

static void set_last_result(void) {
    g_last_stage = g_selected_stage;
    g_last_score = score;
    g_last_blocks_used = blocks_used;
}

static void spawn_and_check_gameover(void) {
    spawn();
    drop_acc_ms = 0;
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
    lc_acc_ms = 0;
    g_mode = GAME_LINECLEAR;
    drop_acc_ms = 0;
    return true;
}

static void enter(void) {
    drop_interval_ms = stage_to_gravity_ms(g_selected_stage);
    g_mode = GAME_PLAY;
    g_lc.count = 0;
    g_lc.frame = 0;
    lc_acc_ms = 0;

    // 재시작 시 보드 잔상/이전 판 상태 제거
    memset(board, 0, sizeof(board));
    drop_acc_ms = 0;
    g_overlay = OVERLAY_NONE;

    // 7-bag은 판 시작 시점에서 리셋 (재시작도 동일)
    bag_reset();
    refill_next_queue();
    spawn();

    pause_cursor = 0;

    hold_type = -1;
    hold_used_this_turn = false;

    score = 0;
    if (is_infinite_mode()) {
        goal = -1;        // 무한모드: goal 사용 안 함
        lines_total = 0;
        level = 1;
        drop_interval_ms = stage_to_gravity_ms(level);
    } else {
        goal = stage_goal_score(g_selected_stage);
        lines_total = 0;
        level = 0;
    }

    blocks_used = 0;
}

// ===== render split =====
static void build_game_view(GameView* v)
{
    memset(v, 0, sizeof(*v));

    v->board = board;

    v->cur_y = y;
    v->cur_x = x;
    v->cur_type = type;
    v->cur_rot  = rot;

    v->ghost_enabled = false;
    v->ghost_y = y;
    if (g_mode == GAME_PLAY && g_settings.ghost) {
        int gy = y;
        while (!check_collision(gy + 1, x, type, rot)) gy++;
        if (gy != y) {
            v->ghost_enabled = true;
            v->ghost_y = gy;
        }
    }

    for (int i = 0; i < NEXT_COUNT; i++) v->next_queue[i] = next_queue[i];

    v->hold_enabled = g_settings.hold;
    v->hold_type = hold_type;

    v->infinite_mode = is_infinite_mode();
    v->stage = g_selected_stage;
    v->score = score;
    v->goal  = goal;
    v->level = level;
    v->lines_total = lines_total;

    v->mode = (g_mode == GAME_LINECLEAR) ? GAMEVIEW_LINECLEAR : GAMEVIEW_PLAY;
    v->lc_count = g_lc.count;
    v->lc_frame = g_lc.frame;
    for (int i = 0; i < g_lc.count; i++) v->lc_rows[i] = g_lc.rows[i];
}

static void render_overlay(void)
{
    Gui* gui = term_get_gui();
    if (g_overlay != OVERLAY_PAUSE) return;

    if (gui) {
        int w=0,h=0; gui_get_size(gui,&w,&h);
        int lh = gui_text_height(gui);

        GuiColor bg    = (GuiColor){20,20,25,235};
        GuiColor bd    = (GuiColor){200,200,210,255};
        GuiColor text  = (GuiColor){235,235,245,255};
        GuiColor hi_bg = (GuiColor){230,230,230,255};
        GuiColor hi_fg = (GuiColor){30,30,35,255};

        int box_w=360, box_h=lh*6+28;
        int x0=(w-box_w)/2, y0=(h-box_h)/2;

        gui_fill_rect(gui,(GuiRect){x0,y0,box_w,box_h},bg);
        gui_draw_rect(gui,(GuiRect){x0,y0,box_w,box_h},bd);

        const char* title="PAUSED";
        gui_draw_text(gui, x0+(box_w-gui_text_width(gui,title))/2, y0+12, text, title);

        const char* menu[3]={"Resume","Restart","Main Menu"};
        for(int i=0;i<3;i++){
            int yy=y0+12+lh*2+i*lh, xx=x0+40, mw=gui_text_width(gui,menu[i]);
            if(i==pause_cursor){ gui_fill_rect(gui,(GuiRect){xx-12,yy-2,mw+24,lh+4},hi_bg); gui_draw_text(gui,xx,yy,hi_fg,menu[i]); }
            else gui_draw_text(gui,xx,yy,text,menu[i]);
        }
        return;
    }

#ifndef BUILD_SDL
    // ncurses 퍼즈는 기존 코드 유지(필요하면 여기 넣기)
#endif
}

static void render(void)
{
    GameView v;
    build_game_view(&v);
    draw_game(&v);
    render_overlay();
}

// scene_game.c
static void handle_pause_input(int ch) {
    if (ch == IK_UP || ch == 'w' || ch == 'W') {
        pause_cursor = (pause_cursor + 2) % 3;
        return;
    }
    if (ch == IK_DOWN || ch == 's' || ch == 'S') {
        pause_cursor = (pause_cursor + 1) % 3;
        return;
    }
    if (ch == IK_CANCEL) {
        g_overlay = OVERLAY_NONE;
        return;
    }
    if (ch == IK_CONFIRM  || ch == '\n' || ch == ' ') {
        if (pause_cursor == 0) g_overlay = OVERLAY_NONE;
        else if (pause_cursor == 1) scene_set(&g_scene_game);
        else scene_set(&g_scene_title);
    }
}

static void handle_play_input(int ch) {
    if (g_mode == GAME_LINECLEAR) return;

    if (ch == 'p' || ch == 'P') {
        g_overlay = OVERLAY_PAUSE;
        pause_cursor = 0;
        return;
    }

    if ((ch == IK_LEFT  || ch == g_settings.key_left)  &&
        !check_collision(y, x - 1, type, rot)) x--;

    if ((ch == IK_RIGHT || ch == g_settings.key_right) &&
        !check_collision(y, x + 1, type, rot)) x++;

    if ((ch == IK_DOWN  || ch == g_settings.key_down)  &&
        !check_collision(y + 1, x, type, rot)) y++;

    if (ch == IK_UP || ch == g_settings.key_rotate) {
        int next = (rot + 1) % 4;
        if (!check_collision(y, x, type, next)) rot = next;
    }

    // hard drop (스페이스 고정)
    if (ch == ' ') {
        while (!check_collision(y + 1, x, type, rot)) y++;
        blocks_used++;
        freeze_block(y, x, type, rot);
        hold_used_this_turn = false;

        if (begin_lineclear_anim_if_needed()) return;
        spawn_and_check_gameover();
        return;
    }

    // hold (C 고정)
    if (g_settings.hold) {
        if ((ch == 'c' || ch == 'C') && !hold_used_this_turn) {
            hold_used_this_turn = true;

            if (hold_type < 0) {
                hold_type = type;
                type = pop_next();
            } else {
                int tmp = type;
                type = hold_type;
                hold_type = tmp;
            }

            reset_active_pos();
            if (check_collision(y, x, type, rot)) {
                set_last_result();
                scene_set(&g_scene_gameover);
            }
        }
    }
}

static void handle_input(int ch) {
    if (g_overlay == OVERLAY_PAUSE) {
        handle_pause_input(ch);
        return;
    }
    handle_play_input(ch);
}

static void update(int dt_ms) {
    // dt 폭주 방지(창 드래그/디버그 등)
    if (dt_ms < 0) dt_ms = 0;
    if (dt_ms > 50) dt_ms = 50;

    if (g_mode == GAME_LINECLEAR) {
        lc_acc_ms += dt_ms;

        while (lc_acc_ms >= LINECLEAR_FRAME_MS) {
            lc_acc_ms -= LINECLEAR_FRAME_MS;
            g_lc.frame++;

            if (g_lc.frame >= LINECLEAR_FRAMES) {
                lc_acc_ms = 0;

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
                return;
            }
        }
        return;
    }

    if (g_overlay == OVERLAY_PAUSE) return;

    // ===== gravity (ms accumulator) =====
    drop_acc_ms += dt_ms;
    if (drop_interval_ms < 30) drop_interval_ms = 30; // 안전 하한

    while (drop_acc_ms >= drop_interval_ms) {
        drop_acc_ms -= drop_interval_ms;

        if (!check_collision(y + 1, x, type, rot)) {
            y++;
            continue;
        }

        // 착지
        blocks_used++;
        freeze_block(y, x, type, rot);
        hold_used_this_turn = false;

        if (begin_lineclear_anim_if_needed()) {
            drop_acc_ms = 0;
            return;
        }
        spawn_and_check_gameover();
        drop_acc_ms = 0;
        return;
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