#pragma once
#define RECORD_MAX 10

typedef struct RecordEntry {
    int stage;
    int score;
    int blocks_used;
} RecordEntry;

void records_init(void);
void records_push(int stage, int score, int blocks_used); // 링버퍼 + 파일 append

int  records_count(void);
const RecordEntry* records_get_latest(int idx);

// ===== log (무한 저장/페이지 표시용) =====
long records_log_count(void); // 파일에 저장된 총 판 수
int  records_log_read_latest_range(long start_from_latest, int n, RecordEntry* out);
// start_from_latest=0이면 가장 최신부터
void records_load(void);

