#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include <stdbool.h>
#include "config.h"

// draw로 넘기는 "읽기 전용 스냅샷" (scene_game에서 채움)
typedef enum {
    GAMEVIEW_PLAY = 0,
    GAMEVIEW_LINECLEAR = 1,
} GameViewMode;

typedef struct {
    int (*board)[BOARD_WIDTH];

    int cur_y, cur_x, cur_type, cur_rot;

    bool ghost_enabled;
    int  ghost_y;

    int next_queue[5];

    bool hold_enabled;
    int  hold_type;

    bool infinite_mode;
    int  stage;
    int  score;
    int  goal;
    int  level;
    int  lines_total;

    GameViewMode mode;
    int lc_rows[4];
    int lc_count;
    int lc_frame;
} GameView;

#endif
