#include <stdio.h>
#include <string.h>

#include "scene.h"
#include "scene_manager.h"
#include "settings.h"

// SDL/GUI 추상화
#include "term_compat.h"
#include "gui.h"

typedef enum {
    ITEM_RANDOMIZER = 0,
    ITEM_GHOST,
    ITEM_HOLD,
    ITEM_WASD,
    ITEM_RESET,
    ITEM_BACK,
    ITEM_COUNT
} Item;

static int cursor = 0;

static void enter(void) { /* noop */ }
static void exit_(void) { /* noop */ }

static void apply_left_right(int dir) {
    // dir: -1(LEFT), +1(RIGHT)
    switch ((Item)cursor) {
        case ITEM_RANDOMIZER:
            g_settings.randomizer = (g_settings.randomizer == RNG_7BAG) ? RNG_PURE : RNG_7BAG;
            break;
        case ITEM_GHOST:
            g_settings.ghost = !g_settings.ghost;
            break;
        case ITEM_HOLD:
            g_settings.hold = !g_settings.hold;
            break;
        case ITEM_WASD:
            g_settings.wasd = (g_settings.wasd == WASD_ON) ? WASD_OFF : WASD_ON;
            break;
        case ITEM_RESET:
            settings_set_defaults();
            break;
        default:
            (void)dir;
            break;
    }
}

// ===== forward =====
static void render_sdl(Gui* gui);
#ifndef BUILD_SDL
static void render_ncu(void);
#endif

static void render(void) {
    Gui* gui = term_get_gui();
    if (gui) { render_sdl(gui); return; }

}

static void render_sdl(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int lh = gui_text_height(gui);
    const int pad = 10;

    GuiColor title_c = (GuiColor){ 235,235,245,255 };
    GuiColor text_c  = (GuiColor){ 230,230,230,255 };
    GuiColor dim_c   = (GuiColor){ 180,180,190,255 };
    GuiColor hi_bg   = (GuiColor){ 230,230,230,255 };
    GuiColor hi_fg   = (GuiColor){ 30,30,35,255 };

    const char* title = "SETTINGS";
    int tw = gui_text_width(gui, title);
    int tx = (w - tw) / 2; if (tx < 0) tx = 0;
    int ty = pad;
    gui_draw_text(gui, tx, ty, title_c, title);

    const char* labels[ITEM_COUNT] = {
        "Randomizer",
        "Ghost",
        "Hold",
        "WASD Enable",
        "Reset to Default",
        "Back"
    };

    int list_top = ty + lh * 3;
    int list_count = ITEM_COUNT;
    int list_h = list_count * lh;
    int centered = (h - list_h) / 2;
    if (centered > list_top) list_top = centered;

    char valuebuf[64];
    for (int i = 0; i < ITEM_COUNT; i++) {
        const char* val = "";

        if (i == ITEM_RANDOMIZER) {
            snprintf(valuebuf, sizeof(valuebuf), "%s", settings_randomizer_name(g_settings.randomizer));
            val = valuebuf;
        } else if (i == ITEM_GHOST) {
            snprintf(valuebuf, sizeof(valuebuf), "%s", settings_onoff_name(g_settings.ghost));
            val = valuebuf;
        } else if (i == ITEM_HOLD) {
            snprintf(valuebuf, sizeof(valuebuf), "%s", settings_onoff_name(g_settings.hold));
            val = valuebuf;
        } else if (i == ITEM_WASD) {
            snprintf(valuebuf, sizeof(valuebuf), "%s", settings_wasd_name(g_settings.wasd));
            val = valuebuf;
        } else {
            val = "";
        }

        char line[128];
        if (val[0] != '\0') snprintf(line, sizeof(line), "%-14s : %s", labels[i], val);
        else                snprintf(line, sizeof(line), "%s", labels[i]);

        int lw = gui_text_width(gui, line);
        int x0 = (w - lw) / 2; if (x0 < 0) x0 = 0;
        int y0 = list_top + i * lh;

        if (i == cursor) {
            gui_fill_rect(gui, (GuiRect){ x0 - pad, y0 - 2, lw + pad*2, lh + 4 }, hi_bg);
            gui_draw_text(gui, x0, y0, hi_fg, line);
        } else {
            gui_draw_text(gui, x0, y0, text_c, line);
        }
    }

    const char* hint = "[UP/DOWN] Select   [LEFT/RIGHT or ENTER] Toggle   [S] Save   [B/ESC] Back";
    int hw = gui_text_width(gui, hint);
    int hx = (w - hw) / 2; if (hx < 0) hx = 0;
    gui_draw_text(gui, hx, h - pad - lh, dim_c, hint);
}

// scene_settings.c
static void handle_input(int ch) {
    switch (ch) {
        case IK_UP:
            cursor = (cursor - 1 + ITEM_COUNT) % ITEM_COUNT;
            break;
        case IK_DOWN:
            cursor = (cursor + 1) % ITEM_COUNT;
            break;

        case IK_LEFT:
        case IK_RIGHT:
            if (cursor != ITEM_BACK) apply_left_right((ch == IK_LEFT) ? -1 : +1);
            break;

        case IK_CONFIRM:
        case '\n':
        case ' ':
            if (cursor == ITEM_BACK) {
                settings_save();
                scene_set(&g_scene_title);
            } else if (cursor == ITEM_RESET) {
                settings_set_defaults();
            } else {
                apply_left_right(+1);
            }
            break;

        case 's':
        case 'S':
            settings_save();
            break;

        case 'b':
        case 'B':
        case IK_CANCEL:
            settings_save();
            scene_set(&g_scene_title);
            break;
    }
}

static void update(int dt_ms) { (void)dt_ms; }

Scene g_scene_settings = {
    .name = "settings",
    .enter = enter,
    .exit = exit_,
    .update = update,
    .render = render,
    .handle_input = handle_input
};