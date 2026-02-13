#include "common.h"

int main() {
    srand((unsigned int)time(NULL));
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100); // 0.1초마다 루프를 강제 실행 (입력이 없어도 진행)
    init_colors();

    int y = 0, x = BOARD_WIDTH / 2 - 2;
    int type = rand() % BLOCK_KIND;; // I-Mino
    int rotation = 0;
    
    int timer = 0; // 블록이 천천히 떨어지게 만드는 타이머

    while (1) {
        clear();
        draw_board(board);              // 쌓인 블록들 그리기
        draw_block(y, x, type, rotation); // 현재 조종 중인 블록 그리기
        refresh();

        int ch = getch();
        if (ch == 'q') break;

        // 1. 키 입력 처리
        if (ch == KEY_LEFT && !check_collision(y, x - 1, type, rotation)) x--;
        if (ch == KEY_RIGHT && !check_collision(y, x + 1, type, rotation)) x++;
        if (ch == KEY_DOWN && !check_collision(y + 1, x, type, rotation)) y++;
        if (ch == KEY_UP) {
            int next_rot = (rotation + 1) % 4;
            if (!check_collision(y, x, type, next_rot)) rotation = next_rot;
        }
        if (ch == ' ' && !check_collision(y + 1, x, type, rotation)) {
            while (!check_collision(y + 1, x, type, rotation)) {
                y++;
            }
            freeze_block(y, x, type, rotation);
        }


        // 2. 자동 하강 로직 (약 0.5초마다 한 칸씩)
        timer++;
        if (timer > 5) { 
            if (!check_collision(y + 1, x, type, rotation)) {
                y++;
            } else {
                // 바닥에 닿으면 고정
                freeze_block(y, x, type, rotation);
                int cleared = clear_lines();
                    if (cleared > 0) {
                        // 점수를 올리거나 효과음
                    }
                // 새로운 블록 생성 (리셋)
                y = 0;
                x = BOARD_WIDTH / 2 - 2;
                type = rand() % BLOCK_KIND;
                rotation = 0;

                // 새 블록이 나오자마자 충돌하면 게임 오버
                if (check_collision(y, x, type, rotation)) {
                    mvprintw(BOARD_HEIGHT / 2, 5, "GAME OVER!");
                    refresh();
                    sleep(2);
                    break;
                }
            }
            timer = 0;
        }
    }

    endwin();
    return 0;
}
