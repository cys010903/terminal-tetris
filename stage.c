// stage.c
#include "stage.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static const char* kClearFile = "clears.log";

// 내부 전용: 1..NUMBER_OF_STAGES
static uint8_t g_cleared[NUMBER_OF_STAGES + 1];

static int clamp_stage(int s)
{
    if (s < 1) return 1;
    if (s > NUMBER_OF_STAGES) return NUMBER_OF_STAGES;
    return s;
}

static void append_clear_log(int stage)
{
    FILE* f = fopen(kClearFile, "a");
    if (!f) return;
    // records.c 쪽 포맷과 맞춤 (#CLEAR stage=.. ts=..)
    // ts는 없어도 되지만, 있으면 디버깅 편함.
    fprintf(f, "#CLEAR stage=%d\n", stage);
    fclose(f);
}

void stage_clear_load(void)
{
    memset(g_cleared, 0, sizeof(g_cleared));

    FILE* f = fopen(kClearFile, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, (int)sizeof(line), f)) {
        if (strncmp(line, "#CLEAR", 6) != 0) continue;

        int st = 0;
        // "#CLEAR stage=3 ..." 형태면 다 잡힘
        if (sscanf(line, "#CLEAR stage=%d", &st) == 1) {
            if (st >= 1 && st <= NUMBER_OF_STAGES) g_cleared[st] = 1;
        }
    }

    fclose(f);
}

int stage_is_cleared(int stage)
{
    stage = clamp_stage(stage);
    return g_cleared[stage] ? 1 : 0;
}

void stage_mark_cleared(int stage)
{
    stage = clamp_stage(stage);

    // 이미 클리어면 중복 기록 안 함
    if (g_cleared[stage]) return;

    g_cleared[stage] = 1;
    append_clear_log(stage);
}