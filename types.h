#ifndef TYPES_H
#define TYPES_H

/*
 * types.h
 * - 프로젝트에서 공유하는 타입/상수(EMPTY 등)
 */

#define EMPTY 0

typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_EXIT
} GameState;

#endif /* TYPES_H */
