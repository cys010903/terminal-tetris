// input_keys.h
#pragma once

// 문자 입력은 ASCII 그대로(32~126)
// 특수키는 ASCII 범위를 피해서 256부터 사용
typedef enum InputKey {
    IK_NONE = 0,

    IK_UP    = 256,
    IK_DOWN  = 257,
    IK_LEFT  = 258,
    IK_RIGHT = 259,

    IK_CONFIRM = 260,
    IK_CANCEL  = 261,

    IK_ENTER = IK_CONFIRM,
    IK_ESC   = IK_CANCEL,
} InputKey;

static inline int ik_is_ascii(int k) { return (k >= 32 && k <= 126); }