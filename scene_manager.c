#include "scene_manager.h"

#include "term_compat.h"
#include "sdl_layout.h"
#include "titlefall.h"

// 타이틀계열 씬 심볼들(프로젝트에 있는 것만 남기세요)
extern Scene g_scene_title;
extern Scene g_scene_stage_select;
extern Scene g_scene_records;
extern Scene g_scene_settings;
extern Scene g_scene_gameover;
extern Scene g_scene_stage_clear;

static int is_title_scene(Scene* s)
{
    return (s == &g_scene_title)
        || (s == &g_scene_stage_select)
        || (s == &g_scene_records)
        || (s == &g_scene_settings)
        || (s == &g_scene_gameover)
        || (s == &g_scene_stage_clear);
}

static TitleFall g_titlefall;
static int g_titlefall_inited = 0;

static Scene* g_scene = 0;
static int g_quit = 0;

void scene_set(Scene* next)
{
    g_quit = 0;

    if (g_scene && g_scene->exit) g_scene->exit();
    g_scene = next;
    if (g_scene && g_scene->enter) g_scene->enter();
}

Scene* scene_get(void) { return g_scene; }

void scene_update(int dt_ms)
{
    if (g_scene && is_title_scene(g_scene)) {
        Gui* gui = term_get_gui();
        if (gui) {
            if (!g_titlefall_inited) {
                int w=0,h=0; gui_get_size(gui,&w,&h);
                int cw=18,ch=18; sdl_layout_get_cell_px(&cw,&ch);
                titlefall_init(&g_titlefall, w, h, cw);
                g_titlefall_inited = 1;
            }
            titlefall_update(&g_titlefall, dt_ms);
        }
    }

    if (g_scene && g_scene->update) g_scene->update(dt_ms);
}

void scene_render(void)
{
    // 타이틀계열 씬이면, 씬 렌더 전에 배경 먼저
    if (g_scene && is_title_scene(g_scene)) {
        Gui* gui = term_get_gui();
        if (gui) {
            int w=0,h=0; gui_get_size(gui,&w,&h);

            if (!g_titlefall_inited) {
                int cw=18,ch=18; sdl_layout_get_cell_px(&cw,&ch);
                titlefall_init(&g_titlefall, w, h, cw);
                g_titlefall_inited = 1;
            } else {
                titlefall_resize(&g_titlefall, w, h);
            }

            titlefall_render(gui, &g_titlefall);
        }
    }

    if (g_scene && g_scene->render) g_scene->render();
}

void scene_input(int ch)
{
    if (g_scene && g_scene->handle_input) g_scene->handle_input(ch);
}

void scene_request_quit(void) { g_quit = 1; }
int scene_is_quit_requested(void) { return g_quit; }