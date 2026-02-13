/* test.c */
#include "common.h"

// draw.c에 만든 함수 선언
void init_colors();
void draw_block(int y, int x, int type, int rotation);

int main() {
    setlocale(LC_ALL, "");
    initscr();
    init_colors(); // 색상 사용 시작
    curs_set(0);   // 커서 숨김

    // I-Mino(0번), 0도 회전 상태를 (5, 10) 좌표에 그려봄
    draw_block(5, 10, 0, 0); 
    
    // T-Mino(5번), 1단계 회전 상태를 (10, 10) 좌표에 그려봄
    draw_block(10, 10, 5, 1);

    mvprintw(20, 10, "Press any key to exit...");
    refresh();
    getch();
    endwin();
    return 0;
}
