#ifndef PLATFORM_H
#define PLATFORM_H

/*
 * platform.h
 * - 프로젝트 전역에서 공통으로 쓰는 시스템/표준 헤더 모음
 * - ncurses, locale 등 "플랫폼/런타임" 의존 요소를 한 곳에서 관리합니다.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* 특수 문자(■) 출력 및 키 입력을 위해 */
#include <locale.h>
#include <ncurses.h>

#endif /* PLATFORM_H */
