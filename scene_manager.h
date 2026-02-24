#pragma once

#include "scene.h"

#include "term_compat.h"
// =============================================================
// Global Scene objects (defined in each scene_*.c)
//
// 규칙:
// - 다른 파일에서 참조해야 하는 전역(Scene)은 절대 static으로 만들지 않습니다.
// - 선언은 여기 한 곳(scene_manager.h)에 모아서, .c 파일에 extern을 흩뿌리지 않습니다.
// =============================================================
extern Scene g_scene_title;
extern Scene g_scene_stage_select;
extern Scene g_scene_game;
extern Scene g_scene_stage_clear;
extern Scene g_scene_gameover;
extern Scene g_scene_records;
extern Scene g_scene_settings;

void scene_set(Scene* next);
Scene* scene_get(void);

void scene_update(int dt_ms);
void scene_render(void);
void scene_input(int ch);

void scene_request_quit(void);
int  scene_is_quit_requested(void);