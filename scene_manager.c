#include "scene_manager.h"

static Scene* g_scene = 0;

void scene_set(Scene* next) {
    if (g_scene && g_scene->exit) g_scene->exit();
    g_scene = next;
    if (g_scene && g_scene->enter) g_scene->enter();
}

Scene* scene_get(void) { return g_scene; }

void scene_update(int dt_ms) {
    if (g_scene && g_scene->update) g_scene->update(dt_ms);
}

void scene_render(void) {
    if (g_scene && g_scene->render) g_scene->render();
}

void scene_input(int ch) {
    if (g_scene && g_scene->handle_input) g_scene->handle_input(ch);
}