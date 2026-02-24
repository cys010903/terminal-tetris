// scene.h
#pragma once
typedef struct AppContext AppContext;

typedef struct Scene {
    const char* name;
    void (*enter)(AppContext* ctx);
    void (*update)(AppContext* ctx, int dt);
    void (*render)(AppContext* ctx);
    void (*handle_input)(AppContext* ctx, int ch);
    void (*exit)(AppContext* ctx);
} Scene;

void scene_set(Scene* next);

extern Scene g_scene_title;
extern Scene g_scene_game;
extern Scene g_scene_stage_select;
extern Scene g_scene_gameover;
extern Scene g_scene_stage_clear;
extern Scene g_scene_records;
extern Scene g_scene_settings;