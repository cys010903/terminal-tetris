#pragma once
#include <stdbool.h>

typedef enum {
    RNG_PURE = 0,   // rand()%7
    RNG_7BAG = 1    // 7-bag
} RandomizerMode;

typedef enum {
    WASD_OFF = 0,
    WASD_ON  = 1
} WasdMode;

typedef struct {
    int version;
    RandomizerMode randomizer;
    bool ghost;
    bool hold;
    WasdMode wasd;
} GameSettings;

extern GameSettings g_settings;

void settings_set_defaults(void);
void settings_load(void);
void settings_save(void);

const char* settings_randomizer_name(RandomizerMode m);
const char* settings_onoff_name(bool v);
const char* settings_wasd_name(WasdMode m);