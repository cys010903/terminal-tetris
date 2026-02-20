#pragma once
#include "types.h"

extern int g_selected_stage;

void scene_set(Scene* next);

extern Scene g_scene_title;
extern Scene g_scene_game;
extern Scene g_scene_stage_select;
extern Scene g_scene_gameover;
extern Scene g_scene_stage_clear;
extern Scene g_scene_records;