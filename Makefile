CC := gcc

TARGET := main
BUILD  := build

# ===== SDL (필요한 것만) =====
SDL_PKGS := sdl2 SDL2_ttf
# SDL2_image 쓰는 프로젝트면 자동으로 링크/플래그 추가(IMG_Load 등)
ifneq ($(shell pkg-config --exists SDL2_image && echo yes),)
SDL_PKGS += SDL2_image
endif

CFLAGS   := -std=c11 -Wall -Wextra -g -D_REENTRANT -DBUILD_SDL
CPPFLAGS := -Icore -Iapp -Iplatform -Ipersist -Ishared
SDL_CFLAGS := $(shell pkg-config --cflags $(SDL_PKGS))
SDL_LIBS   := $(shell pkg-config --libs $(SDL_PKGS))

LDFLAGS :=
LDLIBS  := $(SDL_LIBS) -lm

# ===== Sources =====
SRCS := main_sdl.c $(shell find core app platform persist shared -maxdepth 1 -name '*.c' -print)
OBJS := $(patsubst %.c,$(BUILD)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# build/ 아래에 폴더 구조 유지해서 .o 생성
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(TARGET)

run: $(TARGET)
	./$(TARGET)