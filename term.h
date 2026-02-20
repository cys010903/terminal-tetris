#ifndef TERM_H
#define TERM_H

#include "platform.h"
#include "config.h"

// common.h에 이미 선언돼있던 API를 "정의"할 모듈
int  check_terminal_size(void);

// 작을 때 안내 화면 출력
void render_resize_prompt(void);

#endif