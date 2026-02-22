CC = gcc
CFLAGS = -Wall -g

# ncurses (임시: scene들이 mvprintw 등 ncurses를 직접 써서 링크 필요)
NCURSES_LIBS = -lncursesw

# SDL2
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS   = $(shell sdl2-config --libs)
SDL_TTF    = -lSDL2_ttf

all: term

# ===== 공용(게임 로직/씬) =====
SRCS_GAME = tetris.c stage.c settings.c records.c \
            scene_manager.c scene_title.c scene_stage_select.c \
            scene_game.c scene_gameover.c scene_stage_clear.c \
            scene_records.c scene_settings.c

OBJS_GAME = $(SRCS_GAME:.c=.o)

# ===== 터미널 빌드 =====
SRCS_TERM = main.c draw.c term.c
OBJS_TERM = $(SRCS_TERM:.c=.o)

term: $(OBJS_TERM) $(OBJS_GAME)
	$(CC) $^ -o main $(NCURSES_LIBS)

# ===== SDL 빌드 =====
SRCS_SDL = main_sdl.c gui_sdl.c draw_sdl.c term_compat.c
OBJS_SDL = $(SRCS_SDL:.c=.o)

sdl: $(OBJS_SDL) $(OBJS_GAME)
	$(CC) $^ -o tetris_gui $(SDL_LIBS) $(SDL_TTF) $(NCURSES_LIBS)

# ===== rules =====
%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

# SDL 전용 오브젝트만 SDL_CFLAGS 추가
main_sdl.o gui_sdl.o draw_sdl.o term_compat.o: CFLAGS += $(SDL_CFLAGS)

clean:
	rm -f main tetris_gui *.o

.PHONY: all term sdl clean