#pragma once
#include <stdint.h>
#include "types.h"

void records_init(void);
void records_push(int stage, int score, int blocks_used);
long records_log_count(void);
int  records_log_read_latest_range(long start_from_latest, int n, RecordEntry* out);
int records_read_latest_page(int page, int page_size, RecordEntry* out);

//그 뭐냐 클리어 표시
void records_load(void);
void records_log_append_stage_clear(int stage);
void records_load_stage_clears(uint8_t* cleared, int cleared_bytes);
// 종료 직전 1회: dirty면 파일 저장
void records_flush(void);