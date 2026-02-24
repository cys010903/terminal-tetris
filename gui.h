#ifndef GUI_H
#define GUI_H

#include <stdint.h>

typedef struct {
    int x, y, w, h;
} GuiRect;

typedef struct {
    unsigned char r, g, b, a;
} GuiColor;

// SDL 전파 차단: Gui는 불투명 타입(opaque)으로만 노출
typedef struct Gui Gui;

// SDL_Texture 전파 차단
typedef struct GuiTexture GuiTexture;

typedef struct {
    int quit;
    int last_key;
} GuiInput;

/* lifecycle */
Gui* gui_create(int w, int h, const char* title);
void gui_destroy(Gui* gui);

/* draw */
void gui_fill_rect(Gui* gui, GuiRect r, GuiColor c);
void gui_draw_rect(Gui* gui, GuiRect r, GuiColor c);

void gui_set_bg(Gui* gui, GuiColor c);
void gui_begin_frame(Gui* gui);
void gui_end_frame(Gui* gui);

/* input / time */
uint32_t gui_ticks_ms(void);
void gui_sleep_ms(uint32_t ms);
void gui_poll_input(Gui* gui, GuiInput* in);

/* size */
void gui_get_size(Gui* gui, int* out_w, int* out_h);

/* text */
int  gui_text_init(Gui* gui, const char* font_path, int size);
void gui_text_shutdown(Gui* gui);
int  gui_text_height(Gui* gui);
int  gui_text_width(Gui* gui, const char* utf8);
void gui_draw_text(Gui* gui, int x, int y, GuiColor c, const char* utf8);

/* image (SDL2_image) */
GuiTexture* gui_load_texture(Gui* gui, const char* path, int* out_w, int* out_h);
void        gui_draw_texture(Gui* gui, GuiTexture* tex, GuiRect dst);
void        gui_destroy_texture(GuiTexture** tex);

#endif