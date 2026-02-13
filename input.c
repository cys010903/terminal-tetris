#include "common.h"

int handle_input() {
    int ch = getch();

    switch (ch) {
        case KEY_LEFT:
            return 'l'; // 왼쪽 이동 신호
        case KEY_RIGHT:
            return 'r'; // 오른쪽 이동 신호
        case KEY_DOWN:
            return 'd'; // 소프트 드롭 (빨리 내리기)
        case KEY_UP:
            return 'u'; // 회전 (위 화살표를 회전으로 많이 씁니다)
        case ' ':
            return 's'; // 하드 드롭 (한번에 내리기)
        case 'q':
        case 'Q':
            return 'q'; // 종료 혹은 타이틀로 이동
        default:
            return 0;   // 아무 입력 없음
    }
}
