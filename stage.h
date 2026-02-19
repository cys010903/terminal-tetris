#pragma once
#include "config.h"   // NUMBER_OF_STAGES 쓰려고 필요

// 스테이지 선택 결과(1부터)
extern int g_selected_stage;

// 스테이지별 목표 점수(원하시면 값은 마음대로 조정하세요)
static inline int stage_goal_score(int stage) {
    static const int goals[NUMBER_OF_STAGES + 1] = {
        0,
        500,   // stage 1
        800,   // stage 2
        1200,  // stage 3
        1700,  // stage 4
        2300,  // stage 5
        3000,  // stage 6
        3800,  // stage 7
        4700,  // stage 8
        5700,  // stage 9
        6800   // stage 10
    };

    if (stage < 1) stage = 1;
    if (stage > NUMBER_OF_STAGES) stage = NUMBER_OF_STAGES;
    return goals[stage];
}