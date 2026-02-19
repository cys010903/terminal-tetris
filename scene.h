#pragma once

typedef struct Scene Scene;

struct Scene {
    const char* name;

    // 씬 진입/퇴장 시 한 번
    void (*enter)(void);
    void (*exit)(void);

    // 매 프레임 호출
    void (*update)(int dt_ms);   // 논리 진행 (dt는 대충 100ms 넣어도 됨)
    void (*render)(void);        // 화면 출력
    void (*handle_input)(int ch);// 키 입력 처리
};