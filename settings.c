#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SETTINGS_FILE "settings.cfg"
#define SETTINGS_VERSION 1

GameSettings g_settings;

static void trim(char* s) {
    if (!s) return;
    // right trim
    int n = (int)strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))) {
        s[--n] = '\0';
    }
    // left trim
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i > 0) memmove(s, s + i, strlen(s + i) + 1);
}

static int streq_ci(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

static bool parse_bool(const char* v, bool* out) {
    if (!v || !out) return false;
    if (streq_ci(v, "1") || streq_ci(v, "true") || streq_ci(v, "on") || streq_ci(v, "yes")) { *out = true;  return true; }
    if (streq_ci(v, "0") || streq_ci(v, "false")|| streq_ci(v, "off")|| streq_ci(v, "no"))  { *out = false; return true; }
    return false;
}

void settings_set_defaults(void) {
    g_settings.version = SETTINGS_VERSION;
    g_settings.randomizer = RNG_7BAG; // 기본: 7-bag
    g_settings.ghost = true;
    g_settings.hold = true;
    g_settings.wasd = WASD_OFF;
}

const char* settings_randomizer_name(RandomizerMode m) {
    return (m == RNG_7BAG) ? "7-BAG" : "PURE";
}
const char* settings_onoff_name(bool v) { return v ? "ON" : "OFF"; }
const char* settings_wasd_name(WasdMode m) { return (m == WASD_ON) ? "ON" : "OFF"; }

void settings_load(void) {
    settings_set_defaults();

    FILE* f = fopen(SETTINGS_FILE, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = line;
        char* val = eq + 1;
        trim(key);
        trim(val);

        if (streq_ci(key, "randomizer")) {
            if (streq_ci(val, "7bag") || streq_ci(val, "7-bag")) g_settings.randomizer = RNG_7BAG;
            else g_settings.randomizer = RNG_PURE;
        } else if (streq_ci(key, "ghost")) {
            bool b; if (parse_bool(val, &b)) g_settings.ghost = b;
        } else if (streq_ci(key, "hold")) {
            bool b; if (parse_bool(val, &b)) g_settings.hold = b;
        } else if (streq_ci(key, "wasd")) {
            bool b; if (parse_bool(val, &b)) g_settings.wasd = b ? WASD_ON : WASD_OFF;
        }
    }

    fclose(f);
}

void settings_save(void) {
    FILE* f = fopen(SETTINGS_FILE, "w");
    if (!f) return;

    fprintf(f, "# terminal-tetris settings\n");
    fprintf(f, "version=%d\n", SETTINGS_VERSION);
    fprintf(f, "randomizer=%s\n", (g_settings.randomizer == RNG_7BAG) ? "7bag" : "pure");
    fprintf(f, "ghost=%d\n", g_settings.ghost ? 1 : 0);
    fprintf(f, "hold=%d\n", g_settings.hold ? 1 : 0);
    fprintf(f, "wasd=%d\n", (g_settings.wasd == WASD_ON) ? 1 : 0);

    fclose(f);
}