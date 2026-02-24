// app/scene_manager.c
#include "scene_manager.h"
#include "app_context.h"
#include "titlefx.h"

static Scene* g_scene = 0;
static int g_quit = 0;

static AppContext* g_ctx;
static void ensure_ctx(void){ if(!g_ctx) g_ctx = app_ctx(); }

void scene_set(Scene* next)
{
    ensure_ctx();

    // (선택) 새 씬 들어갈 때 quit 플래그 초기화
    g_quit = 0;

    if (g_scene && g_scene->exit) g_scene->exit(g_ctx);

    g_scene = next;

    if (g_scene && g_scene->enter) g_scene->enter(g_ctx);
}

void scene_update(int dt_ms)
{
    ensure_ctx();

    // ✅ 공통 배경 연출 업데이트
    titlefx_update(dt_ms);

    if (g_scene && g_scene->update) g_scene->update(g_ctx, dt_ms);
}

void scene_render(void)
{
    ensure_ctx();

    // ✅ 공통 배경 연출 렌더(가장 먼저!)
    titlefx_render();

    if (g_scene && g_scene->render) g_scene->render(g_ctx);
}

void scene_input(int ch)
{
    ensure_ctx();
    if (g_scene && g_scene->handle_input) g_scene->handle_input(g_ctx, ch);
}

void scene_request_quit(void)
{
    g_quit = 1;
}

int scene_should_quit(void)
{
    return g_quit;
}