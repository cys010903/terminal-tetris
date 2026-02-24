// input_sdl.c
#include "input_sdl.h"

#include <SDL2/SDL.h>
#include "input_keys.h"

InputKey ik_from_sdl_key(SDL_Keycode sym)
{
    switch (sym) {
    case SDLK_LEFT:  return IK_LEFT;
    case SDLK_RIGHT: return IK_RIGHT;
    case SDLK_DOWN:  return IK_DOWN;
    case SDLK_UP:    return IK_UP;

    case SDLK_ESCAPE: return IK_CANCEL;
    case SDLK_RETURN: return IK_CONFIRM;
    case SDLK_SPACE:  return (InputKey)' ';

    // 문자 키는 그냥 문자로 전달 (씬에서 바인드/wasd 처리 가능)
    case SDLK_a: return (InputKey)'a';
    case SDLK_d: return (InputKey)'d';
    case SDLK_s: return (InputKey)'s';
    case SDLK_w: return (InputKey)'w';
    case SDLK_c: return (InputKey)'c';
    case SDLK_p: return (InputKey)'p';

    default:
        // 알파넘은 그대로 넘겨서 씬에서 비교 가능하게
        if (sym >= 32 && sym <= 126) return (InputKey)sym;
        return IK_NONE;
    }
}