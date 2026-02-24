// input_sdl.h
#pragma once

#ifdef BUILD_SDL
#include <SDL2/SDL.h>
#include "input_keys.h"
int ik_from_sdl_key(SDL_Keycode sym);
#endif