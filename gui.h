#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    int x, y, w, h;
} GuiRect;

typedef struct {
    unsigned char r, g, b, a;
} GuiColor;

typedef struct Gui {
    SDL_Window* win;
    SDL_Renderer* ren;
    TTF_Font* font;
    GuiColor bg;
    int font_h;

    // ===== logical resolution (fixed virtual screen) =====
    // All draw coordinates are in this space.
    // The renderer output is letterboxed to preserve aspect ratio.
    int logical_w;
    int logical_h;
} Gui;

typedef struct {
    int quit;
    int last_key;
} GuiInput;

/* lifecycle */
int gui_init(Gui* gui, int w, int h, const char* title);
void gui_shutdown(Gui* gui);

/* draw */
void gui_clear(Gui* gui, GuiColor c);
void gui_present(Gui* gui);
void gui_fill_rect(Gui* gui, GuiRect r, GuiColor c);
void gui_draw_rect(Gui* gui, GuiRect r, GuiColor c);

/* text */
void gui_text_shutdown(Gui* gui);

int  gui_text_init(Gui* gui, const char* font_path, int size);
void gui_set_bg(Gui* gui, GuiColor c);
void term_bind_gui(Gui* gui);
uint32_t gui_ticks_ms(void);
void gui_poll_input(Gui* gui, GuiInput* in);
void gui_begin_frame(Gui* gui);
void gui_end_frame(Gui* gui);
void gui_sleep_ms(uint32_t ms);

// size / rect
void gui_get_size(Gui* gui, int* out_w, int* out_h);
void gui_fill_rect(Gui* gui, GuiRect r, GuiColor c);
// text
int  gui_text_height(Gui* gui);
int  gui_text_width(Gui* gui, const char* utf8);
void gui_draw_text(Gui* gui, int x, int y, GuiColor c, const char* utf8);

/* image (SDL2_image) */
SDL_Texture* gui_load_texture(Gui* gui, const char* path, int* out_w, int* out_h);
void         gui_draw_texture(Gui* gui, SDL_Texture* tex, GuiRect dst);
void         gui_destroy_texture(SDL_Texture** tex);

#endif