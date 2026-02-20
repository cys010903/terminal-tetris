#include <ncursesw/ncurses.h>
#include <string.h>

#include "scene.h"
#include "scene_manager.h"
#include "settings.h"

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

static void render(void) {
    erase();
    int h, w;
    getmaxyx(stdscr, h, w);

    const char* title = "=== SETTINGS ===";
    mvprintw(2, (w - (int)strlen(title)) / 2, "%s", title);

    int y = 5;
    const char* labels[ITEM_COUNT] = {
        "Randomizer",
        "Ghost",
        "Hold",
        "WASD Enable",
        "Reset to Default",
        "Back"
    };

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

        int x0 = (w - (int)strlen(line)) / 2;
        if (x0 < 0) x0 = 0;

        if (i == cursor) attron(A_REVERSE);
        mvprintw(y + i, x0, "%s", line);
        if (i == cursor) attroff(A_REVERSE);
    }

    const char* hint = "[UP/DOWN] Select   [LEFT/RIGHT or ENTER] Toggle   [S] Save   [B/ESC] Back";
    mvprintw(h - 2, (w - (int)strlen(hint)) / 2, "%s", hint);

    refresh();
}

static void handle_input(int ch) {
    switch (ch) {
        case KEY_UP:
            cursor = (cursor - 1 + ITEM_COUNT) % ITEM_COUNT;
            break;
        case KEY_DOWN:
            cursor = (cursor + 1) % ITEM_COUNT;
            break;
        case KEY_LEFT:
        case KEY_RIGHT:
            if (cursor != ITEM_BACK) apply_left_right((ch == KEY_LEFT) ? -1 : +1);
            break;

        case '\n':
        case KEY_ENTER:
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
        case 27: // ESC
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