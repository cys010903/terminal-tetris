#pragma once
#include "types.h"

void records_init(void);
void records_push(int stage, int score, int blocks_used);
long records_log_count(void);
int  records_log_read_latest_range(long start_from_latest, int n, RecordEntry* out);
int records_read_latest_page(int page, int page_size, RecordEntry* out);
void records_load(void);
void records_log_append_stage_clear(int stage);