// scene_manager.h
#pragma once

#include "scene.h"

// 씬 전환
void scene_set(Scene* next);

// 메인루프에서 호출
void scene_update(int dt_ms);
void scene_render(void);
void scene_input(int ch);

// 종료 플래그
void scene_request_quit(void);
int  scene_should_quit(void);