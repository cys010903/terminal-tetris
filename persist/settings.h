// settings.h
#pragma once
#include <stdbool.h>
#include "input_keys.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RNG_PURE = 0,
    RNG_7BAG = 1
} RandomizerMode;

typedef enum {
    WASD_OFF = 0,
    WASD_ON  = 1
} WasdMode;

typedef struct GameSettings{
    int version;
    RandomizerMode randomizer;
    bool ghost;
    bool hold;
    WasdMode wasd;

    InputKey key_left;
    InputKey key_right;
    InputKey key_down;
    InputKey key_rotate;

    InputKey key_page_prev;
    InputKey key_page_next;
    InputKey key_back;
} GameSettings;

void settings_set_defaults(GameSettings* s);
void settings_load(GameSettings* s);
void settings_save(const GameSettings* s);

const char* settings_randomizer_name(RandomizerMode m);
const char* settings_onoff_name(bool v);
const char* settings_wasd_name(WasdMode m);

#ifdef __cplusplus
}
#endif