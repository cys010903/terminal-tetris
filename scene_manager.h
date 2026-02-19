#pragma once
#include "scene.h"

void scene_set(Scene* next);
Scene* scene_get(void);

void scene_update(int dt_ms);
void scene_render(void);
void scene_input(int ch);