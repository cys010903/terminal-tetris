# 1. 컴파일러 및 옵션 설정
CC = gcc
CFLAGS = -Wall -g   # -g는 나중에 디버깅할 때 필요합니다.
LDFLAGS = -lncursesw

# 2. 최종 실행 파일 이름
TARGET = main

# 3. 컴파일에 필요한 모든 소스 파일들
SRCS = main.c draw.c tetris.c scene_stage_select.c \
       scene_manager.c scene_title.c scene_game.c scene_gameover.c scene_stage_clear.c
# 소스 파일(.c) 이름을 오브젝트 파일(.o) 이름으로 자동 변환
OBJS = $(SRCS:.c=.o)

# 4. 기본 규칙 (make 실행 시)
all: $(TARGET)

# 5. 최종 타겟 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 6. 각 .c 파일을 .o 파일로 만드는 규칙
%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

# 7. 정리 규칙 (make clean)
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
