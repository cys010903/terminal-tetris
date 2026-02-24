# Makefile (SDL-only)

CC     := gcc
TARGET := main

SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS   := $(shell sdl2-config --libs   2>/dev/null)

TTF_CFLAGS := $(shell pkg-config --cflags SDL2_ttf 2>/dev/null)
TTF_LIBS   := $(shell pkg-config --libs   SDL2_ttf 2>/dev/null)

IMG_CFLAGS := $(shell pkg-config --cflags SDL2_image 2>/dev/null)
IMG_LIBS   := $(shell pkg-config --libs   SDL2_image 2>/dev/null)

CFLAGS := -Wall -g -D_REENTRANT -DBUILD_SDL $(SDL_CFLAGS) $(TTF_CFLAGS) $(IMG_CFLAGS)
LDLIBS := $(SDL_LIBS) $(TTF_LIBS) $(IMG_LIBS) -lm

SRCS := \
  main_sdl.c gui_sdl.c term_compat.c sdl_layout.c \
  input_sdl.c \
  scene_manager.c scene_title.c scene_stage_select.c scene_game.c \
  scene_gameover.c scene_stage_clear.c scene_records.c scene_settings.c \
  tetris.c records.c stage.c settings.c common.c layout.c draw_game.c \
  draw_sdl.c bg_crt.c titlefall.c app_context.c

OBJS := $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)