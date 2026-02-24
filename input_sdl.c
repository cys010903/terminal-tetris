// input_sdl.c
#include "input_sdl.h"

#ifdef BUILD_SDL
int ik_from_sdl_key(SDL_Keycode sym)
{
    switch (sym) {
    case SDLK_UP:    return IK_UP;
    case SDLK_DOWN:  return IK_DOWN;
    case SDLK_LEFT:  return IK_LEFT;
    case SDLK_RIGHT: return IK_RIGHT;

    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        return IK_CONFIRM;

    case SDLK_ESCAPE:
        return IK_CANCEL;

    default:
        if (sym >= 32 && sym <= 126) return (int)sym; // ASCII
        return IK_NONE;
    }
}
#endif