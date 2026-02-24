// input_sdl.h
#pragma once

#include <SDL2/SDL.h>
#include "input_keys.h"
InputKey ik_from_sdl_key(SDL_Keycode sym);
