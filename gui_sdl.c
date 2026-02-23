#include "gui.h"
#include "term_compat.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void apply_logical_letterbox(Gui* gui)
{
    if (!gui || !gui->ren || !gui->win) return;
    if (gui->logical_w <= 0 || gui->logical_h <= 0) return;

    int win_w = 0, win_h = 0;
    SDL_GetWindowSize(gui->win, &win_w, &win_h);
    if (win_w <= 0 || win_h <= 0) return;

    const float sx = (float)win_w / (float)gui->logical_w;
    const float sy = (float)win_h / (float)gui->logical_h;
    const float s  = (sx < sy) ? sx : sy;

    const int vp_w = (int)(gui->logical_w * s + 0.5f);
    const int vp_h = (int)(gui->logical_h * s + 0.5f);

    SDL_Rect vp;
    vp.x = (win_w - vp_w) / 2;
    vp.y = (win_h - vp_h) / 2;
    vp.w = vp_w;
    vp.h = vp_h;

    SDL_RenderSetViewport(gui->ren, &vp);
    SDL_RenderSetScale(gui->ren, s, s);
}

int gui_init(Gui* gui, int w, int h, const char* title)
{
    if (!gui) return 0;
    *gui = (Gui){0};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        return 0;
    }

    if (TTF_Init() != 0) {
        SDL_Quit();
        return 0;
    }

    // 창 리사이즈 허용 (논리 해상도에 맞춰 레터박싱)
    SDL_Window* win = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        SDL_WINDOW_RESIZABLE
    );
    if (!win) {
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    gui->win = win;
    gui->ren = ren;
    gui->bg  = (GuiColor){ 20, 20, 24, 255 };
    gui->font = NULL;
    gui->font_h = 0;

    // logical resolution = init size (virtual screen)
    gui->logical_w = w;
    gui->logical_h = h;

    apply_logical_letterbox(gui);
    return 1;
}

void gui_shutdown(Gui* gui)
{
    if (!gui) return;

    gui_text_shutdown(gui);

    if (gui->ren) SDL_DestroyRenderer(gui->ren);
    if (gui->win) SDL_DestroyWindow(gui->win);

    TTF_Quit();
    SDL_Quit();
}

void gui_poll_input(Gui* gui, GuiInput* in)
{
    (void)gui;
    if (!in) return;

    in->quit = 0;
    in->last_key = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            in->quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) in->quit = 1;
            in->last_key = (int)e.key.keysym.sym;
        }
    }
}

void gui_set_bg(Gui* gui, GuiColor c)
{
    if (!gui) return;
    gui->bg = c;
}

void gui_begin_frame(Gui* gui)
{
    if (!gui) return;

    // Clear whole window (including letterbox area)
    SDL_RenderSetViewport(gui->ren, NULL);
    SDL_RenderSetScale(gui->ren, 1.0f, 1.0f);
    SDL_SetRenderDrawColor(gui->ren, gui->bg.r, gui->bg.g, gui->bg.b, gui->bg.a);
    SDL_RenderClear(gui->ren);

    // Then apply letterboxed logical viewport for all subsequent draw calls.
    apply_logical_letterbox(gui);
}

void gui_end_frame(Gui* gui)
{
    if (!gui) return;
    SDL_RenderPresent(gui->ren);
}

void gui_get_size(Gui* gui, int* out_w, int* out_h)
{
    if (!gui) return;
    // Return logical size so layout is stable and never "falls out" of the window.
    if (out_w) *out_w = gui->logical_w;
    if (out_h) *out_h = gui->logical_h;
}

static SDL_Rect to_sdl_rect(GuiRect r)
{
    SDL_Rect sr;
    sr.x = r.x; sr.y = r.y; sr.w = r.w; sr.h = r.h;
    return sr;
}

void gui_fill_rect(Gui* gui, GuiRect r, GuiColor c)
{
    if (!gui) return;
    SDL_SetRenderDrawColor(gui->ren, c.r, c.g, c.b, c.a);
    SDL_Rect sr = to_sdl_rect(r);
    SDL_RenderFillRect(gui->ren, &sr);
}

void gui_draw_rect(Gui* gui, GuiRect r, GuiColor c)
{
    if (!gui) return;
    SDL_SetRenderDrawColor(gui->ren, c.r, c.g, c.b, c.a);
    SDL_Rect sr = to_sdl_rect(r);
    SDL_RenderDrawRect(gui->ren, &sr);
}

static const char* pick_default_font_path(void)
{
    static const char* candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
    };
    for (size_t i = 0; i < sizeof(candidates)/sizeof(candidates[0]); i++) {
        FILE* fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

int  gui_text_init(Gui* gui, const char* font_path, int pt_size)
{
    if (!gui) return 0;

    gui_text_shutdown(gui);

    if (!font_path) font_path = pick_default_font_path();
    if (!font_path) return 0;

    gui->font = TTF_OpenFont(font_path, pt_size);
    if (!gui->font) return 0;

    gui->font_h = TTF_FontHeight(gui->font);
    return 1;
}

void gui_text_shutdown(Gui* gui)
{
    if (!gui) return;
    if (gui->font) {
        TTF_CloseFont(gui->font);
        gui->font = NULL;
    }
}

int gui_text_height(Gui* gui)
{
    if (!gui || !gui->font) return 16;
    return gui->font_h;
}

int gui_text_width(Gui* gui, const char* utf8)
{
    if (!gui || !gui->font || !utf8) return 0;
    int w = 0, h = 0;
    if (TTF_SizeUTF8(gui->font, utf8, &w, &h) != 0) return 0;
    return w;
}

void gui_draw_text(Gui* gui, int x, int y, GuiColor c, const char* utf8)
{
    if (!gui || !gui->font || !utf8) return;

    SDL_Color sc = { c.r, c.g, c.b, c.a };
    SDL_Surface* surf = TTF_RenderUTF8_Blended(gui->font, utf8, sc);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(gui->ren, surf);
    if (!tex) { SDL_FreeSurface(surf); return; }

    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_FreeSurface(surf);

    SDL_RenderCopy(gui->ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

uint32_t gui_ticks_ms(void)
{
    return (uint32_t)SDL_GetTicks();
}

void gui_sleep_ms(uint32_t ms)
{
    SDL_Delay(ms);
}

SDL_Renderer* gui_sdl_get_renderer(Gui* gui)
{
    if (!gui) return NULL;
    return gui->ren;
}