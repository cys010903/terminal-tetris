// stage.c
#include "stage.h"

// 스테이지 클리어 여부(1~NUMBER_OF_STAGES)
bool g_stage_cleared[NUMBER_OF_STAGES + 1] = { false };