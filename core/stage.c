// stage.c
#include "stage.h"
#include "records.h"
#include <string.h>
#include <stdint.h>

// 내부 전용: 1..NUMBER_OF_STAGES
static uint8_t g_cleared[NUMBER_OF_STAGES + 1];

static int clamp_stage(int s)
{
    if (s < 1) return 1;
    if (s > NUMBER_OF_STAGES) return NUMBER_OF_STAGES;
    return s;
}

void stage_clear_load(void)
{
    memset(g_cleared, 0, sizeof(g_cleared));

    // 파일 I/O는 records.c가 담당
    // clears.log를 읽어서 g_cleared[] 채움
    records_load_stage_clears(g_cleared, (int)sizeof(g_cleared));
}

int stage_is_cleared(int stage)
{
    stage = clamp_stage(stage);
    return g_cleared[stage] ? 1 : 0;
}

void stage_mark_cleared(int stage)
{
    stage = clamp_stage(stage);

    // 메모리 상태만 갱신 (중복 방지)
    if (g_cleared[stage]) return;
    g_cleared[stage] = 1;

    // 저장/로그는 씬에서 records_log_append_stage_clear(...)로 수행
}