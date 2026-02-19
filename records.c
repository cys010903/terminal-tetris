#include "records.h"
#include <stdio.h>
#include <string.h>


typedef struct {
    RecordEntry buf[RECORD_MAX];
    int head;
    int count;
} RecordRing;

static RecordRing g_ring;
static const char* kLogFile = "records.log";

void records_init(void) { memset(&g_ring, 0, sizeof(g_ring)); }

static int phys_from_oldest_index(int oldest_index){
    return (g_ring.head + oldest_index) % RECORD_MAX;
}

static void log_append(const RecordEntry* e){
    FILE* f = fopen(kLogFile, "ab");
    if (!f) return;
    fwrite(e, sizeof(*e), 1, f);
    fclose(f);
}

void records_push(int stage, int score, int blocks_used){
    RecordEntry e = { stage, score, blocks_used };

    // 링버퍼 push
    if (g_ring.count < RECORD_MAX) {
        int tail = phys_from_oldest_index(g_ring.count);
        g_ring.buf[tail] = e;
        g_ring.count++;
    } else {
        g_ring.buf[g_ring.head] = e;
        g_ring.head = (g_ring.head + 1) % RECORD_MAX;
    }

    // 무한 로그 append
    log_append(&e);
}

int records_count(void){ return g_ring.count; }

const RecordEntry* records_get_latest(int idx){
    if (idx < 0 || idx >= g_ring.count) return NULL;
    int oldest_index = (g_ring.count - 1 - idx);
    int phys = phys_from_oldest_index(oldest_index);
    return &g_ring.buf[phys];
}

long records_log_count(void){
    FILE* f = fopen(kLogFile, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long sz = ftell(f);
    fclose(f);
    if (sz < 0) return 0;
    return sz / (long)sizeof(RecordEntry);
}

// 최신부터 start_from_latest만큼 건너뛰고 n개 읽기
int records_log_read_latest_range(long start_from_latest, int n, RecordEntry* out){
    if (n <= 0) return 0;

    long total = records_log_count();
    if (total <= 0) return 0;
    if (start_from_latest < 0) start_from_latest = 0;
    if (start_from_latest >= total) return 0;

    FILE* f = fopen(kLogFile, "rb");
    if (!f) return 0;

    int got = 0;
    for (int i = 0; i < n; i++) {
        long latest_index = (total - 1) - (start_from_latest + i); // 파일상의 인덱스(0=첫판)
        if (latest_index < 0) break;

        long off = latest_index * (long)sizeof(RecordEntry);
        if (fseek(f, off, SEEK_SET) != 0) break;

        if (fread(&out[got], sizeof(RecordEntry), 1, f) != 1) break;
        got++;
    }

    fclose(f);
    return got;
}

void records_load(void){
    // 로그 파일에서 마지막 RECORD_MAX개를 읽어서 링버퍼를 복원
    memset(&g_ring, 0, sizeof(g_ring));

    long total = records_log_count();
    if (total <= 0) return;

    int n = (total < RECORD_MAX) ? (int)total : RECORD_MAX;

    FILE* f = fopen(kLogFile, "rb");
    if (!f) return;

    long start = total - n; // 파일에서 읽기 시작할 (오래된 쪽) 인덱스
    long off = start * (long)sizeof(RecordEntry);
    if (fseek(f, off, SEEK_SET) != 0) { fclose(f); return; }

    // buf[0]이 가장 오래된, buf[n-1]이 가장 최신이 되도록 채움
    size_t got = fread(g_ring.buf, sizeof(RecordEntry), (size_t)n, f);
    fclose(f);

    g_ring.head  = 0;
    g_ring.count = (int)got;
}

