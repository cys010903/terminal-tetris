// app_context.c
#include "app_context.h"

static AppContext g_ctx = { .selected_stage = 1 };

AppContext* app_ctx(void) { return &g_ctx; }

void app_set_last_result(int stage, int score, int blocks_used)
{
    g_ctx.last_stage = stage;
    g_ctx.last_score = score;
    g_ctx.last_blocks_used = blocks_used;
}