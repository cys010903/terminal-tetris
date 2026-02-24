// scene_manager.c
#include "scene_manager.h"
#include "app_context.h"

static Scene* g_scene = 0;
static int g_quit = 0;

// AppContext는 scene_manager 한 곳에서만 소유/획득
static AppContext* g_ctx = 0;

static AppContext* ctx_get(void)
{
    if (!g_ctx) g_ctx = app_ctx();
    return g_ctx;
}

void scene_set(Scene* next)
{
    AppContext* ctx = ctx_get();

    // (선택) 새 씬 들어갈 때 quit 플래그 초기화
    g_quit = 0;

    if (g_scene && g_scene->exit) {
        g_scene->exit(ctx);
    }

    g_scene = next;

    if (g_scene && g_scene->enter) {
        g_scene->enter(ctx);
    }
}

void scene_update(int dt_ms)
{
    AppContext* ctx = ctx_get();
    if (!g_scene || !g_scene->update) return;
    g_scene->update(ctx, dt_ms);
}

void scene_render(void)
{
    AppContext* ctx = ctx_get();
    if (!g_scene || !g_scene->render) return;
    g_scene->render(ctx);
}

void scene_input(int ch)
{
    AppContext* ctx = ctx_get();
    if (!g_scene || !g_scene->handle_input) return;
    g_scene->handle_input(ctx, ch);
}

void scene_request_quit(void)
{
    g_quit = 1;
}

int scene_should_quit(void)
{
    return g_quit;
}