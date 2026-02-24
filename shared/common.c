#include "common.h"

int clampi(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}