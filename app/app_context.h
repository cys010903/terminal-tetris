// app_context.h
#pragma once
#include "settings.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct AppContext {
    int selected_stage;

    int last_stage;
    int last_score;
    int last_blocks_used;
    GameSettings settings;
} AppContext;

AppContext* app_ctx(void);
void app_set_last_result(int stage, int score, int blocks_used);

#ifdef __cplusplus
}
#endif