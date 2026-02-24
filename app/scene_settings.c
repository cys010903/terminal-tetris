#include <stdio.h>
#include <string.h>

#include "scene.h"
#include "scene_manager.h"
#include "settings.h"
#include "app_context.h" 
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

static GameSettings* settings = NULL;
static int cursor = 0;

static void enter(AppContext* ctx)
{
    settings = &ctx->settings;
}
static void exit_(AppContext* ctx) { (void)ctx; }

static void apply_left_right(int dir)
{
    (void)dir;

    switch ((Item)cursor) {
    case ITEM_RANDOMIZER:
        settings->randomizer = (settings->randomizer == RNG_7BAG) ? RNG_PURE : RNG_7BAG;
        break;
    case ITEM_GHOST:
        settings->ghost = !settings->ghost;
        break;
    case ITEM_HOLD:
        settings->hold = !settings->hold;
        break;
    case ITEM_WASD:
        settings->wasd = (settings->wasd == WASD_ON) ? WASD_OFF : WASD_ON;
        break;
    case ITEM_RESET:
        settings_set_defaults(settings);   // ✅ 인자 버전
        break;
    default:
        break;
    }
}

// ===== forward =====
static void render_sdl(Gui* gui);

static void render(AppContext* ctx) { (void)ctx;
    Gui* gui = term_get_gui();
    if (gui) {
        render_sdl(gui);
        return;
    }
}

static void render_sdl(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int lh = gui_text_height(gui);
    const int pad = 10;

    const int inner_pad = 14;
    const int line_step = lh + 4;

    GuiColor title_c = (GuiColor){ 235,235,245,255 };
    GuiColor text_c  = (GuiColor){ 230,230,230,255 };
    GuiColor dim_c   = (GuiColor){ 180,180,190,255 };
    GuiColor hi_bg   = (GuiColor){ 230,230,230,255 };
    GuiColor hi_fg   = (GuiColor){ 30,30,35,255 };

    // title
    {
        const char* title = "SETTINGS";
        int tw = gui_text_width(gui, title);
        int tx = (w - tw) / 2; if (tx < 0) tx = 0;
        int ty = pad;
        gui_draw_text(gui, tx, ty, title_c, title);
    }

    const char* labels[ITEM_COUNT] = {
        "Randomizer",
        "Ghost",
        "Hold",
        "WASD Enable",
        "Reset to Default",
        "Back"
    };

    // ===== 라인 미리 구성 + 최대폭 계산 =====
    char lines[ITEM_COUNT][128];
    int maxw = 0;

    for (int i = 0; i < ITEM_COUNT; i++) {
        char valuebuf[64];
        const char* val = "";

        if (i == ITEM_RANDOMIZER) {
            snprintf(valuebuf, sizeof(valuebuf), "%s",
                    settings_randomizer_name(settings->randomizer));
            val = valuebuf;
        } else if (i == ITEM_GHOST) {
            snprintf(valuebuf, sizeof(valuebuf), "%s",
                    settings_onoff_name(settings->ghost));
            val = valuebuf;
        } else if (i == ITEM_HOLD) {
            snprintf(valuebuf, sizeof(valuebuf), "%s",
                    settings_onoff_name(settings->hold));
            val = valuebuf;
        } else if (i == ITEM_WASD) {
            snprintf(valuebuf, sizeof(valuebuf), "%s",
                    settings_wasd_name(settings->wasd));
            val = valuebuf;
        }

        if (val[0] != '\0') snprintf(lines[i], sizeof(lines[i]), "%-14s : %s", labels[i], val);
        else                snprintf(lines[i], sizeof(lines[i]), "%s", labels[i]);

        int ww = gui_text_width(gui, lines[i]);
        if (ww > maxw) maxw = ww;
    }

    const char* hint = "[UP/DOWN] Select   [LEFT/RIGHT or ENTER] Toggle   [S] Save   [B/ESC] Back";
    int hw = gui_text_width(gui, hint);
    if (hw > maxw) maxw = hw;

    // ===== 패널(텍스트 감싸기) =====
    int panel_w = maxw + inner_pad * 2;
    if (panel_w > w - 24) panel_w = w - 24;

    int panel_h = inner_pad * 2 + ((ITEM_COUNT + 1) * line_step) - 4; // items + hint
    if (panel_h > h - 24) panel_h = h - 24;

    int px = (w - panel_w) / 2;
    int py = pad + lh + 18;
    if (py + panel_h > h - 12) py = h - 12 - panel_h;
    if (py < pad + lh + 10) py = pad + lh + 10;

    GuiColor panel_bg = (GuiColor){ 40,40,48,235 };
    GuiColor panel_bd = (GuiColor){ 180,180,190,255 };

    gui_fill_rect(gui, (GuiRect){ px, py, panel_w, panel_h }, panel_bg);
    gui_draw_rect(gui, (GuiRect){ px, py, panel_w, panel_h }, panel_bd);

    // ===== 패널 내부 출력 =====
    int x_text = px + inner_pad;
    int y0 = py + inner_pad;

    for (int i = 0; i < ITEM_COUNT; i++) {
        int yy = y0 + i * line_step;
        int lw = gui_text_width(gui, lines[i]);

        if (i == cursor) {
            gui_fill_rect(gui, (GuiRect){ x_text - 8, yy - 2, lw + 16, lh + 4 }, hi_bg);
            gui_draw_text(gui, x_text, yy, hi_fg, lines[i]);
        } else {
            gui_draw_text(gui, x_text, yy, text_c, lines[i]);
        }
    }

    gui_draw_text(gui, x_text, py + panel_h - inner_pad - lh, dim_c, hint);
}

// scene_settings.c
static void handle_input(AppContext* ctx, int ch) { (void) ctx;
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
            settings_save(settings);
            scene_set(&g_scene_title);
        } else if (cursor == ITEM_RESET) {
            settings_set_defaults(settings);
        } else {
            apply_left_right(+1);
        }
        break;

    case 's':
    case 'S':
        settings_save(settings);
        break;

    case 'b':
    case 'B':
    case IK_CANCEL:
        settings_save(settings);
        scene_set(&g_scene_title);
        break;
    }
}

static void update(AppContext* ctx, int dt_ms) { (void)ctx;  (void) dt_ms;  }

Scene g_scene_settings = {
    .name = "settings",
    .enter = enter,
    .exit = exit_,
    .update = update,
    .render = render,
    .handle_input = handle_input
};