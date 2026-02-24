#include "settings.h"
#include "input_keys.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SETTINGS_FILE "settings.cfg"
#define SETTINGS_VERSION 1

static void trim(char* s) {
    if (!s) return;
    int n = (int)strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))) {
        s[--n] = '\0';
    }
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

static const char* ik_name_local(InputKey k) {
    switch (k) {
    case IK_LEFT:   return "LEFT";
    case IK_RIGHT:  return "RIGHT";
    case IK_DOWN:   return "DOWN";
    case IK_UP:     return "UP";
    case IK_CANCEL: return "ESC";
    default:        return "NONE";
    }
}

static bool parse_ik(const char* v, InputKey* out) {
    if (!v || !out) return false;
    if (streq_ci(v, "LEFT"))  { *out = IK_LEFT;   return true; }
    if (streq_ci(v, "RIGHT")) { *out = IK_RIGHT;  return true; }
    if (streq_ci(v, "DOWN"))  { *out = IK_DOWN;   return true; }
    if (streq_ci(v, "UP"))    { *out = IK_UP;     return true; }
    if (streq_ci(v, "ESC"))   { *out = IK_CANCEL; return true; }
    if (streq_ci(v, "NONE"))  { *out = IK_NONE;   return true; }
    return false;
}

const char* settings_randomizer_name(RandomizerMode m) { return (m == RNG_7BAG) ? "7-BAG" : "PURE"; }
const char* settings_onoff_name(bool v) { return v ? "ON" : "OFF"; }
const char* settings_wasd_name(WasdMode m) { return (m == WASD_ON) ? "ON" : "OFF"; }

void settings_set_defaults(GameSettings* s)
{
    if (!s) return;

    s->version = SETTINGS_VERSION;
    s->randomizer = RNG_7BAG;
    s->ghost = true;
    s->hold  = true;
    s->wasd  = WASD_OFF;

    s->key_left   = IK_LEFT;
    s->key_right  = IK_RIGHT;
    s->key_down   = IK_DOWN;
    s->key_rotate = IK_UP;

    s->key_page_prev = IK_LEFT;
    s->key_page_next = IK_RIGHT;
    s->key_back      = IK_CANCEL;
}

void settings_load(GameSettings* s)
{
    if (!s) return;
    settings_set_defaults(s);

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
            if (streq_ci(val, "7bag") || streq_ci(val, "7-bag")) s->randomizer = RNG_7BAG;
            else s->randomizer = RNG_PURE;
        } else if (streq_ci(key, "ghost")) {
            bool b; if (parse_bool(val, &b)) s->ghost = b;
        } else if (streq_ci(key, "hold")) {
            bool b; if (parse_bool(val, &b)) s->hold = b;
        } else if (streq_ci(key, "wasd")) {
            bool b; if (parse_bool(val, &b)) s->wasd = b ? WASD_ON : WASD_OFF;
        } else if (streq_ci(key, "key_left")) {
            InputKey k; if (parse_ik(val, &k)) s->key_left = k;
        } else if (streq_ci(key, "key_right")) {
            InputKey k; if (parse_ik(val, &k)) s->key_right = k;
        } else if (streq_ci(key, "key_down")) {
            InputKey k; if (parse_ik(val, &k)) s->key_down = k;
        } else if (streq_ci(key, "key_rotate")) {
            InputKey k; if (parse_ik(val, &k)) s->key_rotate = k;
        }
    }

    fclose(f);
}

void settings_save(const GameSettings* s)
{
    if (!s) return;

    FILE* f = fopen(SETTINGS_FILE, "w");
    if (!f) return;

    fprintf(f, "# terminal-tetris settings\n");
    fprintf(f, "version=%d\n", SETTINGS_VERSION);
    fprintf(f, "randomizer=%s\n", (s->randomizer == RNG_7BAG) ? "7bag" : "pure");
    fprintf(f, "ghost=%d\n", s->ghost ? 1 : 0);
    fprintf(f, "hold=%d\n", s->hold ? 1 : 0);
    fprintf(f, "wasd=%d\n", (s->wasd == WASD_ON) ? 1 : 0);

    fprintf(f, "key_left=%s\n",   ik_name_local(s->key_left));
    fprintf(f, "key_right=%s\n",  ik_name_local(s->key_right));
    fprintf(f, "key_down=%s\n",   ik_name_local(s->key_down));
    fprintf(f, "key_rotate=%s\n", ik_name_local(s->key_rotate));

    fclose(f);
}