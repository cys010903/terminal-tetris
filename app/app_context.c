#include "app_context.h"

static AppContext g_ctx;

AppContext* app_ctx(void)
{
    static int inited = 0;
    if (!inited) {
        g_ctx.selected_stage = 1;
        settings_set_defaults(&g_ctx.settings);
        settings_load(&g_ctx.settings); // 원하면 로드
        inited = 1;
    }
    return &g_ctx;
}

void app_set_last_result(int stage, int score, int blocks_used)
{
    g_ctx.last_stage = stage;
    g_ctx.last_score = score;
    g_ctx.last_blocks_used = blocks_used;
}