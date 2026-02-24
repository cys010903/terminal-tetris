#pragma once

typedef struct {
    int stage;
    int score;
    int blocks_used;
} RecordEntry;



typedef struct Scene {
    const char* name;
    void (*enter)(void);
    void (*update)(int dt);
    void (*render)(void);
    void (*handle_input)(int ch);
    void (*exit)(void);
} Scene;